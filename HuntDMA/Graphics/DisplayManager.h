#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include "imgui.h"

struct MonitorInfo
{
	HMONITOR hMonitor;
	int x, y;
	int width, height;
	char name[64];
	bool isPrimary;
};

class DisplayManager
{
public:
	static void Initialize();
	static void RefreshMonitors();
	static int GetMonitorCount() { return static_cast<int>(m_Monitors.size()); }
	static const MonitorInfo& GetMonitor(int index);
	static int GetCurrentMonitorIndex() { return m_CurrentMonitor; }
	static void SetCurrentMonitor(int index);

	// Get current monitor offset
	static ImVec2 GetMonitorOffset();
	static int GetMonitorX();
	static int GetMonitorY();

	// Resolution presets
	static void ApplyResolutionPreset(int presetIndex);
	static const char* GetResolutionPresetName(int index);
	static int GetResolutionPresetCount() { return 5; }
	static int GetCurrentResolutionPreset() { return m_CurrentResolutionPreset; }

private:
	static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

public:
	static inline std::vector<MonitorInfo> m_Monitors;
	static inline int m_CurrentMonitor = 0;
	static inline int m_CurrentResolutionPreset = 0;
	static inline float ScreenWidth = 1920.0f;
	static inline float ScreenHeight = 1080.0f;
};
