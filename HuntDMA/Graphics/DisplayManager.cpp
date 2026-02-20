#include "pch.h"
#include "DisplayManager.h"
#include <iostream>

void DisplayManager::Initialize()
{
	RefreshMonitors();

	// Set to primary monitor by default
	if (!m_Monitors.empty())
	{
		for (size_t i = 0; i < m_Monitors.size(); i++)
		{
			if (m_Monitors[i].isPrimary)
			{
				SetCurrentMonitor(static_cast<int>(i));
				return;
			}
		}
		SetCurrentMonitor(0);
	}
}

BOOL CALLBACK DisplayManager::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	MONITORINFOEX mi = {};
	mi.cbSize = sizeof(MONITORINFOEX);
	GetMonitorInfo(hMonitor, &mi);

	MonitorInfo info;
	info.hMonitor = hMonitor;
	info.x = mi.rcMonitor.left;
	info.y = mi.rcMonitor.top;
	info.width = mi.rcMonitor.right - mi.rcMonitor.left;
	info.height = mi.rcMonitor.bottom - mi.rcMonitor.top;
	info.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;

	// Create display name
	int monitorNum = static_cast<int>(m_Monitors.size()) + 1;
	if (info.isPrimary)
	{
		sprintf_s(info.name, "Monitor %d (%dx%d) [Primary]", monitorNum, info.width, info.height);
	}
	else
	{
		sprintf_s(info.name, "Monitor %d (%dx%d)", monitorNum, info.width, info.height);
	}

	m_Monitors.push_back(info);
	return TRUE;
}

void DisplayManager::RefreshMonitors()
{
	m_Monitors.clear();
	EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, 0);

	std::cout << "[Display] Found " << m_Monitors.size() << " monitor(s)" << std::endl;
	for (size_t i = 0; i < m_Monitors.size(); i++)
	{
		std::cout << "  [" << i << "] " << m_Monitors[i].name << std::endl;
	}
}

const MonitorInfo& DisplayManager::GetMonitor(int index)
{
	static MonitorInfo dummy = {};
	if (index < 0 || index >= static_cast<int>(m_Monitors.size()))
		return dummy;
	return m_Monitors[index];
}

void DisplayManager::SetCurrentMonitor(int index)
{
	if (index < 0 || index >= static_cast<int>(m_Monitors.size()))
		return;

	m_CurrentMonitor = index;
	const auto& mon = m_Monitors[index];

	// Update screen size for the project
	ScreenWidth = static_cast<float>(mon.width);
	ScreenHeight = static_cast<float>(mon.height);

	std::cout << "[Display] Switched to " << mon.name << std::endl;
}

void DisplayManager::ApplyResolutionPreset(int presetIndex)
{
	m_CurrentResolutionPreset = presetIndex;

	switch (presetIndex)
	{
	case 0: // 1920x1080
		ScreenWidth = 1920.0f;
		ScreenHeight = 1080.0f;
		break;
	case 1: // 2560x1440
		ScreenWidth = 2560.0f;
		ScreenHeight = 1440.0f;
		break;
	case 2: // 3440x1440 (Ultrawide)
		ScreenWidth = 3440.0f;
		ScreenHeight = 1440.0f;
		break;
	case 3: // 3840x2160 (4K)
		ScreenWidth = 3840.0f;
		ScreenHeight = 2160.0f;
		break;
	case 4: // Custom (do nothing, user will set manually)
		break;
	}

	std::cout << "[Display] Resolution set to " << ScreenWidth << "x" << ScreenHeight << std::endl;
}

const char* DisplayManager::GetResolutionPresetName(int index)
{
	static const char* presets[] = {
		"1920x1080 (Full HD)",
		"2560x1440 (QHD)",
		"3440x1440 (Ultrawide)",
		"3840x2160 (4K)",
		"Custom"
	};

	if (index < 0 || index >= 5)
		return "Unknown";

	return presets[index];
}

ImVec2 DisplayManager::GetMonitorOffset()
{
	if (m_CurrentMonitor < 0 || m_CurrentMonitor >= static_cast<int>(m_Monitors.size()))
		return ImVec2(0.0f, 0.0f);

	return ImVec2(static_cast<float>(m_Monitors[m_CurrentMonitor].x),
		static_cast<float>(m_Monitors[m_CurrentMonitor].y));
}

int DisplayManager::GetMonitorX()
{
	if (m_CurrentMonitor < 0 || m_CurrentMonitor >= static_cast<int>(m_Monitors.size()))
		return 0;

	return m_Monitors[m_CurrentMonitor].x;
}

int DisplayManager::GetMonitorY()
{
	if (m_CurrentMonitor < 0 || m_CurrentMonitor >= static_cast<int>(m_Monitors.size()))
		return 0;

	return m_Monitors[m_CurrentMonitor].y;
}
