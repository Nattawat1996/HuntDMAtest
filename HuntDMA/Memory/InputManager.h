#pragma once
#include "pch.h"
#include "Registry.h"
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <vector>

namespace Keyboard
{
	// Core initialization and lifecycle
	bool InitKeyboard();
	void StartPolling();
	void StopPolling();

	// Key state query
	void UpdateKeys();
	bool IsKeyDown(uint32_t virtual_key_code);
	bool IsKeyPressed(uint32_t virtual_key_code); // Returns true only on fresh press

	// Hotkey system
	enum class HotkeyMode { OnPress, OnRelease, WhileDown };
	
	using HotkeyCallback = std::function<void()>;
	
	void RegisterHotkey(uint32_t vk, HotkeyMode mode, HotkeyCallback callback);
	void RegisterDynamicHotkey(int* vkVar, HotkeyCallback callback); // Checks *vkVar every tick
	void UnregisterHotkey(uint32_t vk);
	void ClearAllHotkeys();

	// Internal helpers
	uint64_t FindSignature(int pid, uint64_t start, uint64_t end, const char* pattern);
};