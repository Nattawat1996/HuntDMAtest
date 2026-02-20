#include "pch.h"
#include "InputManager.h"
#include "Registry.h"
#include "Memory/Memory.h"
#include <chrono>

namespace Keyboard
{
	uint64_t gafAsyncKeyStateExport = 0;
	uint8_t state_bitmap[64]{ };
	uint8_t previous_state_bitmap[64]{ };
	uint64_t win32kbase = 0;
	int win_logon_pid = 0;

	// Threading
	std::thread pollingThread;
	std::atomic<bool> shouldRun{ false };
	std::mutex stateMutex;
	int pollIntervalMs = 8; // 125Hz default

	// Hotkey system
	std::mutex hotkeyMutex;
	std::map<uint32_t, std::vector<std::pair<HotkeyMode, HotkeyCallback>>> hotkeys;
	std::vector<std::pair<int*, HotkeyCallback>> dynamicHotkeys;
}

// Signature scanning helper
uint64_t Keyboard::FindSignature(int pid, uint64_t start, uint64_t end, const char* pattern)
{
	// Parse pattern (supports wildcards with ?)
	std::vector<int> bytes;
	std::vector<bool> mask;
	
	const char* p = pattern;
	while (*p)
	{
		if (*p == ' ') { p++; continue; }
		if (*p == '?')
		{
			bytes.push_back(0);
			mask.push_back(false);
			p += (*(p + 1) == '?') ? 2 : 1;
		}
		else
		{
			char byteStr[3] = { p[0], p[1], 0 };
			bytes.push_back(strtol(byteStr, nullptr, 16));
			mask.push_back(true);
			p += 2;
		}
	}

	if (bytes.empty()) return 0;

	// Read memory in chunks
	constexpr size_t chunkSize = 0x10000;
	std::vector<uint8_t> buffer(chunkSize);
	
	for (uint64_t addr = start; addr < end; addr += chunkSize)
	{
		size_t readSize = (chunkSize < (size_t)(end - addr)) ? chunkSize : (size_t)(end - addr);
		if (!VMMDLL_MemReadEx(TargetProcess.vHandle, pid, addr, buffer.data(), (DWORD)readSize, NULL, VMMDLL_FLAG_NOCACHE))
			continue;

		for (size_t i = 0; i <= readSize - bytes.size(); i++)
		{
			bool found = true;
			for (size_t j = 0; j < bytes.size(); j++)
			{
				if (mask[j] && buffer[i + j] != bytes[j])
				{
					found = false;
					break;
				}
			}
			if (found)
				return addr + i;
		}
	}
	return 0;
}

bool ResolveWin11()
{
	printf("[Keyboard] Win11+ resolver (signature scan)\n");
	
	auto csrss_pids = TargetProcess.GetPidListFromName("csrss.exe");
	if (csrss_pids.empty())
	{
		printf("[Keyboard] csrss.exe not found\n");
		return false;
	}

	for (auto pid : csrss_pids)
	{
		// Get win32k module base
		uintptr_t kBase = VMMDLL_ProcessGetModuleBaseU(TargetProcess.vHandle, pid, (LPSTR)"win32ksgd.sys");
		if (kBase == 0)
			kBase = VMMDLL_ProcessGetModuleBaseU(TargetProcess.vHandle, pid, (LPSTR)"win32k.sys");
		
		if (kBase == 0) continue;

		// Use reasonable scan size for kernel modules (typically ~2-4MB)
		uint64_t kSize = 0x400000; // 4MB scan range

		// Find g_session_global_slots signature
		uint64_t gSessPtr = Keyboard::FindSignature(pid, kBase, kBase + kSize, "48 8B 05 ?? ?? ?? ?? 48 8B 04 C8");
		if (gSessPtr == 0)
			gSessPtr = Keyboard::FindSignature(pid, kBase, kBase + kSize, "48 8B 05 ?? ?? ?? ?? FF C9");
		
		if (gSessPtr == 0)
		{
			printf("[Keyboard] g_session_global_slots signature not found\n");
			continue;
		}

		// Read RIP-relative offset
		int rel = TargetProcess.Read<int>(gSessPtr + 3, pid);
		uint64_t gSlots = gSessPtr + 7 + rel;

		// Walk slots to find user_session_state
		uint64_t userSession = 0;
		for (int i = 0; i < 8; i++)
		{
			uint64_t t1 = TargetProcess.Read<uint64_t>(gSlots, pid);
			uint64_t t2 = TargetProcess.Read<uint64_t>(t1 + (8 * i), pid);
			uint64_t t3 = TargetProcess.Read<uint64_t>(t2, pid);
			userSession = t3;
			if (userSession > 0x7FFFFFFFFFFF) break;
		}

		if (userSession <= 0x7FFFFFFFFFFF)
		{
			printf("[Keyboard] user_session_state not canonical\n");
			continue;
		}

		// Find offset in win32kbase.sys
		uintptr_t kbase2 = VMMDLL_ProcessGetModuleBaseU(TargetProcess.vHandle, pid, (LPSTR)"win32kbase.sys");
		if (kbase2 == 0) continue;

		// Use reasonable scan size for win32kbase.sys
		uint64_t ksz = 0x400000; // 4MB scan range
		uint64_t offsetPtr = Keyboard::FindSignature(pid, kbase2, kbase2 + ksz, "48 8D 90 ?? ?? ?? ?? E8 ?? ?? ?? ?? 0F 57 C0");
		
		if (offsetPtr == 0)
		{
			printf("[Keyboard] Session offset signature not found\n");
			continue;
		}

		uint32_t offset = TargetProcess.Read<uint32_t>(offsetPtr + 3, pid);
		Keyboard::gafAsyncKeyStateExport = userSession + offset;

		if (Keyboard::gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
		{
			printf("[Keyboard] gaf=0x%llX (signature-based)\n", Keyboard::gafAsyncKeyStateExport);
			return true;
		}
	}

	return false;
}

bool Keyboard::InitKeyboard()
{
	c_registry registry;
	std::string win = registry.QueryValue("HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\CurrentBuild", e_registry_type::sz);
	if (win == "")
		return false;
	
	int Winver = 0;
	if (!win.empty())
		Winver = std::stoi(win);
	else
		return false;

	win_logon_pid = TargetProcess.GetPidFromName("winlogon.exe");
	
	// Windows 11+ (Build 22000+)
	if (Winver > 22000)
	{
		return ResolveWin11();
	}
	// Windows 10
	else
	{
		printf("[Keyboard] Win10 resolver (EAT)\n");
		PVMMDLL_MAP_EAT eat_map = NULL;
		PVMMDLL_MAP_EATENTRY eat_map_entry;
		bool result = VMMDLL_Map_GetEATU(TargetProcess.vHandle, TargetProcess.GetPidFromName("winlogon.exe") | VMMDLL_PID_PROCESS_WITH_KERNELMEMORY, (LPSTR)"win32kbase.sys", &eat_map);
		if (!result)
			return false;

		if (eat_map->dwVersion != VMMDLL_MAP_EAT_VERSION)
		{
			VMMDLL_MemFree(eat_map);
			eat_map_entry = NULL;
			return false;
		}
		
		for (int i = 0; i < eat_map->cMap; i++)
		{
			eat_map_entry = eat_map->pMap + i;
			if (strcmp(eat_map_entry->uszFunction, "gafAsyncKeyState") == 0)
			{
				gafAsyncKeyStateExport = eat_map_entry->vaFunction;
				break;
			}
		}

		VMMDLL_MemFree(eat_map);
		eat_map = NULL;
		
		if (gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
		{
			printf("[Keyboard] gaf=0x%llX (EAT)\n", gafAsyncKeyStateExport);
			return true;
		}
		return false;
	}
}

void PollThreadFunction()
{
	printf("[Keyboard] Polling thread started\n");
	
	while (Keyboard::shouldRun)
	{
		Keyboard::UpdateKeys();
		std::this_thread::sleep_for(std::chrono::milliseconds(Keyboard::pollIntervalMs));
	}
	
	printf("[Keyboard] Polling thread stopped\n");
}

void Keyboard::StartPolling()
{
	if (shouldRun)
		return;

	shouldRun = true;
	pollingThread = std::thread(PollThreadFunction);
}

void Keyboard::StopPolling()
{
	if (!shouldRun)
		return;

	shouldRun = false;
	if (pollingThread.joinable())
		pollingThread.join();
}

void Keyboard::UpdateKeys()
{
	if (gafAsyncKeyStateExport < 0x7FFFFFFFFFFF)
		return;

	uint8_t newState[64];
	if (!VMMDLL_MemReadEx(TargetProcess.vHandle, win_logon_pid | VMMDLL_PID_PROCESS_WITH_KERNELMEMORY, 
		gafAsyncKeyStateExport, (PBYTE)&newState, 64, NULL, VMMDLL_FLAG_NOCACHE))
		return;

	// Thread-safe state update
	{
		std::lock_guard<std::mutex> lock(stateMutex);
		memcpy(previous_state_bitmap, state_bitmap, 64);
		memcpy(state_bitmap, newState, 64);
	}

	// Fire hotkeys (outside lock to prevent deadlock)
	std::lock_guard<std::mutex> hkLock(hotkeyMutex);
	for (auto& [vk, callbacks] : hotkeys)
	{
		bool was = (previous_state_bitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2) != 0;
		bool now = (newState[(vk * 2 / 8)] & 1 << vk % 4 * 2) != 0;

		for (auto& [mode, callback] : callbacks)
		{
			try
			{
				if (mode == HotkeyMode::OnPress && !was && now)
					callback();
				else if (mode == HotkeyMode::OnRelease && was && !now)
					callback();
				else if (mode == HotkeyMode::WhileDown && now)
					callback();
			}
			catch (const std::exception& e)
			{
				printf("[Keyboard] Hotkey VK=0x%X error: %s\n", vk, e.what());
			}
		}
	}

	// Process dynamic hotkeys (bound by pointer)
	for (auto& [vkPtr, callback] : dynamicHotkeys)
	{
		if (!vkPtr) continue;
		int vk = *vkPtr;
		if (vk <= 0 || vk > 255) continue;

		bool was = (previous_state_bitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2) != 0;
		bool now = (newState[(vk * 2 / 8)] & 1 << vk % 4 * 2) != 0;

		if (!was && now) // OnPress only for dynamic currently
		{
			try { callback(); }
			catch (...) {}
		}
	}
}

bool Keyboard::IsKeyDown(uint32_t virtual_key_code)
{
	if (gafAsyncKeyStateExport < 0x7FFFFFFFFFFF)
		return false;
	
	std::lock_guard<std::mutex> lock(stateMutex);
	return state_bitmap[(virtual_key_code * 2 / 8)] & 1 << virtual_key_code % 4 * 2;
}

bool Keyboard::IsKeyPressed(uint32_t virtual_key_code)
{
	if (gafAsyncKeyStateExport < 0x7FFFFFFFFFFF)
		return false;

	std::lock_guard<std::mutex> lock(stateMutex);
	bool now = state_bitmap[(virtual_key_code * 2 / 8)] & 1 << virtual_key_code % 4 * 2;
	bool was = previous_state_bitmap[(virtual_key_code * 2 / 8)] & 1 << virtual_key_code % 4 * 2;
	return now && !was;
}

void Keyboard::RegisterHotkey(uint32_t vk, HotkeyMode mode, HotkeyCallback callback)
{
	std::lock_guard<std::mutex> lock(hotkeyMutex);
	hotkeys[vk].push_back({ mode, callback });
}

void Keyboard::RegisterDynamicHotkey(int* vkVar, HotkeyCallback callback)
{
	std::lock_guard<std::mutex> lock(hotkeyMutex);
	dynamicHotkeys.push_back({ vkVar, callback });
}

void Keyboard::UnregisterHotkey(uint32_t vk)
{
	std::lock_guard<std::mutex> lock(hotkeyMutex);
	hotkeys.erase(vk);
}

void Keyboard::ClearAllHotkeys()
{
	std::lock_guard<std::mutex> lock(hotkeyMutex);
	hotkeys.clear();
}