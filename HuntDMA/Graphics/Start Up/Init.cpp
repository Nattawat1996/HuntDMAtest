#include "pch.h"
#include "Globals.h"
#include "PlayerEsp.h"
#include "OtherEsp.h"
#include "ConfigUtilities.h"
#include "Aimbot.h"
#include "InputManager.h"
#include "Kmbox.h"
#include "Overlay.h"
#include <chrono>
#include "SpectatorAlarm.h"
#include "Init.h"
#include "CacheManager.h"

ID2D1Factory* Factory;
IDWriteFactory* FontFactory;
ID2D1HwndRenderTarget* RenderTarget;
ID2D1SolidColorBrush* Brush;


std::shared_ptr<Environment> EnvironmentInstance;
std::shared_ptr<Camera> CameraInstance;

bool cacheThreadCreated = false;

void InitialiseClasses()
{
	EnvironmentInstance = std::make_shared<Environment>();
	CameraInstance = std::make_shared<Camera>();
}

std::shared_ptr<CheatFunction> UpdateCam = std::make_shared<CheatFunction>(1, [] {
	if (EnvironmentInstance == nullptr)
		return;
	if (EnvironmentInstance->GetObjectCount() < 10)
		return;
	auto handle = TargetProcess.CreateScatterHandle();
	CameraInstance->UpdateCamera(handle);
	TargetProcess.ExecuteReadScatter(handle);
	TargetProcess.CloseScatterHandle(handle);
});

void InitializeESP()
{
	g_CacheManager.Start();
	
	// Always initialize keyboard for ESP toggles (reads game PC keyboard via DMA)
	Keyboard::InitKeyboard();
	Keyboard::StartPolling();
	
	// Only initialize KMBox for aimbot output (if aimbot is enabled)
	if (enableAimBot)
	{
		kmbox::KmboxInitialize(Configs.Aimbot.KmboxPort, Configs.Aimbot.KmboxBaudRate, (kmbox::DeviceType)Configs.Aimbot.KmboxDeviceType);
	}
}

int FrameRate()
{
	static int fps = 0;
	static int lastfps = 0;
	static auto lasttime = std::chrono::steady_clock::now();

	auto currenttime = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = currenttime - lasttime;
	fps++;

	if (elapsed.count() >= 1.0f)
	{
		lasttime = currenttime;
		lastfps = fps;
		fps = 0;
	}

	return lastfps;
}