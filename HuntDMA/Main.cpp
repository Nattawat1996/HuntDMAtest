#pragma warning(disable: 4200) // Zero-sized arrays
#pragma warning(disable: 4477) // Format string mismatches
#pragma warning(disable: 4018) // Signed/unsigned mismatch
#pragma warning(disable: 4267) // Data loss in conversion
#pragma warning(disable: 4313) // Format string conflicts

#include "pch.h"
#include "Memory.h"
#include "CheatFunction.h"
#include "Environment.h"
#include "Globals.h"
#include "Camera.h"
#include "ConfigUtilities.h"
#include "Init.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuiMenu.h"
#include "ESPRenderer.h"
#include "SpectatorAlarm.h"
#include "PlayerEsp.h"
#include "OtherEsp.h"
#include "Overlay.h"
#include "Aimbot.h"
#include "SystemInfo.h"
#include "Localization/Localization.h"
#include <windows.h>
#include <algorithm>
#include <climits>
#include <cstdlib>
#include "resource.h"
#include "Graphics/DisplayManager.h"
#include "CacheManager.h"

void InitializeGame()
{
	while (!TargetProcess.Init("HuntGame.exe", true, true))
	{
        LOG_WARNING("Failed to attach to game process. Retrying in 2 seconds...");
		Sleep(2000);
	}
	TargetProcess.FixCr3();
    LOG_INFO("HuntGame base address: 0x%X", TargetProcess.GetBaseAddress("HuntGame.exe"));
    LOG_INFO("GameHunt base address: 0x%X", TargetProcess.GetBaseAddress("GameHunt.dll"));
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool isMouseTracking = false; // To re-enable tracking for mouse window leaving event

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// First pass events to ImGui
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	// Then handle standard messages
	InputWndProc(hWnd, message, wParam, lParam);

	// Additionally handle window messages
	switch (message)
	{
		case WM_DESTROY:
            LOG_WARNING("Window destroying, application shutting down");
            DeleteObject((HBRUSH)GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND));
			PostQuitMessage(0);
			return 0;
		case WM_MOUSEMOVE:
			if (!isMouseTracking) {
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				TrackMouseEvent(&tme);
				isMouseTracking = true;
			}
			break;
		case WM_MOUSELEAVE:
			isMouseTracking = false;
			break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

        // Initialize logger first
        Logger::GetInstance().Init();
        SystemInfo::LogSystemInfo();
        LOG_INFO("Hunt DMA starting...");

        // Create debug console
        if (AllocConsole()) {
            LOG_DEBUG("Debug console created");
            FILE* fDummy;
            freopen_s(&fDummy, "CONIN$", "r", stdin);
            freopen_s(&fDummy, "CONOUT$", "w", stderr);
            freopen_s(&fDummy, "CONOUT$", "w", stdout);
        }
        else {
            LOG_ERROR("Failed to create debug console");
        }

        LOG_INFO("Initializing configurations...");
        SetUpConfig();
        LoadConfig(ConfigPath);

        LOG_INFO("Initializing localization system...");
        Localization::Initialize();

        LOG_INFO("Initializing game connection...");
        InitializeGame();

        PlaySound(MAKEINTRESOURCE(IDR_WAVE1), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);

        // Create window
        WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = L"Hunt DMA";

        if (!RegisterClassEx(&wc)) {
            LOG_CRITICAL("Failed to register window class!");
            return -1;
        }

        DWORD exStyle;
        if (Configs.General.OverlayMode)
            exStyle = WS_EX_TOPMOST |
                      WS_EX_LAYERED |
                      WS_EX_TRANSPARENT |
                      WS_EX_NOACTIVATE |
                      WS_EX_TOOLWINDOW;
        else
            exStyle = WS_EX_APPWINDOW;

        DisplayManager::Initialize();

        HWND hWnd = CreateWindowEx(
            exStyle,
            wc.lpszClassName,
            L"Hunt DMA",
            WS_POPUP,
            DisplayManager::GetMonitorX(), DisplayManager::GetMonitorY(),
            (int)DisplayManager::ScreenWidth, (int)DisplayManager::ScreenHeight,
            NULL, NULL,
            hInstance,
            NULL
        );

        if (!hWnd) {
            LOG_CRITICAL("Failed to create window!");
            return -1;
        }

        LOG_INFO("Window created successfully");

        if (Configs.General.PreventRecording) {
            BOOL status = SetWindowDisplayAffinity(hWnd, WDA_EXCLUDEFROMCAPTURE);
            if (!status) {
                LOG_WARNING("Failed to SetWindowDisplayAffinity to WDA_EXCLUDEFROMCAPTURE");
            }
        }

        if (!SetLayeredWindowAttributes(hWnd, 0, 255, LWA_ALPHA)) {
            LOG_WARNING("Failed to set window transparency");
        }

        BOOL enable = TRUE;
        DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY,
            &enable, sizeof(enable));

        if (Configs.General.OverlayMode)
            SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);

        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);
        if (Configs.General.OverlayMode)
        {
            MARGINS margins = { -1, -1, -1, -1 };
            DwmExtendFrameIntoClientArea(hWnd, &margins);
        }

        LOG_INFO("Initializing game environment...");
        InitialiseClasses();

        LOG_INFO("Detaching caching thread...");
        InitializeESP();

        LOG_INFO("Initializing ImGui...");
        if (!g_ImGuiMenu.Init(hWnd)) {
            LOG_CRITICAL("Failed to initialize ImGui!");
            MessageBox(NULL, L"Failed to initialize ImGui!", L"Error", MB_OK);
            return -1;
        }

        LOG_INFO("Initializing ESP Renderer...");
        if (!ESPRenderer::Initialize()) {
            LOG_CRITICAL("Failed to initialize ESP Renderer!");
            MessageBox(NULL, L"Failed to initialize ESP Renderer!", L"Error", MB_OK);
            return -1;
        }

        LOG_INFO("Setting up input system...");
        SetInput();

        // Main loop
        LOG_INFO("Entering main loop...");
        MSG msg;
        ZeroMemory(&msg, sizeof(msg));
        while (msg.message != WM_QUIT) {
            try {
                if (Configs.General.OverlayMode)
                {
                    static int lastX = INT_MIN;
                    static int lastY = INT_MIN;
                    static int lastW = -1;
                    static int lastH = -1;

                    const int targetX = DisplayManager::GetMonitorX();
                    const int targetY = DisplayManager::GetMonitorY();
                    const int targetW = (int)DisplayManager::ScreenWidth;
                    const int targetH = (int)DisplayManager::ScreenHeight;

                    if (targetX != lastX || targetY != lastY ||
                        targetW != lastW || targetH != lastH) {
                        SetWindowPos(hWnd, HWND_TOPMOST, targetX, targetY, targetW,
                                     targetH, SWP_NOACTIVATE);
                        lastX = targetX;
                        lastY = targetY;
                        lastW = targetW;
                        lastH = targetH;
                    }
                }

                // Process Windows messages
                if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                    continue;
                }

                const bool worldReady = EnvironmentInstance->GetObjectCount() >= 10;

                // ── Render-thread scatter split ──
                // Camera is refreshed every frame (reduces mouse-look ESP delay).
                // Full entity position sync still follows configured ESP Sync Hz.
                if (worldReady) {
                    static auto lastEntitySyncRead =
                        std::chrono::steady_clock::now() - std::chrono::seconds(1);
                    static auto lastSnapshotRefresh =
                        std::chrono::steady_clock::now() - std::chrono::seconds(1);
                    static std::vector<std::shared_ptr<WorldEntity>> playersCache;
                    static std::vector<std::shared_ptr<WorldEntity>> bossesCache;
                    static size_t playerCursor = 0;
                    static size_t bossCursor = 0;
                    static std::weak_ptr<WorldEntity> localPlayerWeak;
                    static uint16_t lastObjectCountSeen = 0;

                    int syncHz = Configs.General.AutoEspSyncHz
                        ? DisplayManager::GetRecommendedEspSyncHz()
                        : Configs.General.EspSyncHz;
                    syncHz = std::clamp(syncHz, 30, 300);
                    const auto kMinSyncInterval =
                        std::chrono::microseconds(1000000 / syncHz);

                    const auto now = std::chrono::steady_clock::now();
                    const bool doEntitySync = (now - lastEntitySyncRead >= kMinSyncInterval);
                    const uint16_t objectCountNow = EnvironmentInstance->GetObjectCount();
                    const int objectCountDelta =
                        std::abs(static_cast<int>(objectCountNow) -
                                 static_cast<int>(lastObjectCountSeen));
                    const bool significantObjectCountChange =
                        (lastObjectCountSeen == 0) || (objectCountDelta >= 96);
                    const bool refreshSnapshots =
                        playersCache.empty() ||
                        significantObjectCountChange ||
                        ((now - lastSnapshotRefresh) >= std::chrono::seconds(20));
                    if (refreshSnapshots)
                    {
                        const auto& rawPlayers = EnvironmentInstance->GetPlayerList();
                        playersCache.clear();
                        playersCache.reserve(rawPlayers.size());
                        for (const auto& p : rawPlayers)
                        {
                            if (!p) continue;
                            const EntityType t = p->GetType();
                            if (t == EntityType::LocalPlayer || t == EntityType::EnemyPlayer ||
                                t == EntityType::FriendlyPlayer)
                                playersCache.push_back(p);
                        }

                        const auto& rawBosses = EnvironmentInstance->GetBossesList();
                        bossesCache.assign(rawBosses.begin(), rawBosses.end());

                        if (!playersCache.empty())
                            playerCursor %= playersCache.size();
                        else
                            playerCursor = 0;

                        if (!bossesCache.empty())
                            bossCursor %= bossesCache.size();
                        else
                            bossCursor = 0;

                        std::shared_ptr<WorldEntity> localPlayer = nullptr;
                        for (const auto& p : playersCache)
                        {
                            if (p && p->GetType() == EntityType::LocalPlayer) {
                                localPlayer = p;
                                break;
                            }
                        }
                        localPlayerWeak = localPlayer;
                        lastSnapshotRefresh = now;
                        lastObjectCountSeen = objectCountNow;
                    }

                    if (auto handle = TargetProcess.CreateScatterHandle()) {
                        // 1) Always refresh camera for the current render frame.
                        CameraInstance->UpdateCamera(handle);

                        // 2) Refresh player positions aggressively (round-robin),
                        // so ESP remains responsive while keeping DMA bursts bounded.
                        std::vector<std::shared_ptr<WorldEntity>> velocityUpdatedPlayers;
                        std::shared_ptr<WorldEntity> localPlayer = localPlayerWeak.lock();
                        if (localPlayer) {
                            localPlayer->UpdatePosition(handle);
                            velocityUpdatedPlayers.push_back(localPlayer);
                        }

                        const size_t playerBudget = std::min<size_t>(32, playersCache.size());
                        for (size_t i = 0; i < playerBudget && !playersCache.empty(); ++i) {
                            if (playerCursor >= playersCache.size()) playerCursor = 0;
                            auto& p = playersCache[playerCursor++];
                            if (!p || p == localPlayer) continue;
                            p->UpdatePosition(handle);
                            velocityUpdatedPlayers.push_back(p);
                        }

                        if (doEntitySync) {
                            const size_t bossBudget = std::min<size_t>(6, bossesCache.size());
                            for (size_t i = 0; i < bossBudget && !bossesCache.empty(); ++i) {
                                if (bossCursor >= bossesCache.size()) bossCursor = 0;
                                auto& b = bossesCache[bossCursor++];
                                if (!b) continue;
                                b->UpdatePosition(handle);
                            }
                        }

                        TargetProcess.ExecuteReadScatter(handle);
                        TargetProcess.CloseScatterHandle(handle);

                        // 3) Publish latest camera state every frame.
                        CameraInstance->CommitUpdate();

                        for (auto& p : velocityUpdatedPlayers) {
                            if (p) p->UpdateVelocity();
                        }

                        if (doEntitySync)
                            lastEntitySyncRead = now;
                    }
                }

                // 6. Freeze a per-frame camera snapshot — WorldToScreen and aimbot
                //    read from this; it cannot change mid-frame.
                CameraInstance->SnapForFrame();

                if (enableAimBot && worldReady) Aimbot();

                // Handle Insert key for menu toggle
                g_ImGuiMenu.HandleInput();

                // Begin render frame
                g_ImGuiMenu.BeginFrame();
                ESPRenderer::BeginFrame();

                // Draw ESP elements
                DrawSpectators();
                DrawRadar();
                DrawOtherEsp();
                DrawBossesEsp();
                DrawPlayersEsp();
                DrawOverlay();

                // Enable/Disable cousor
                if (Configs.General.OverlayMode)
                {
                    if (MenuOpen) {
                        SetForegroundWindow(hWnd);
                        if (exStyle & WS_EX_TRANSPARENT) {
                            exStyle &= ~WS_EX_TRANSPARENT;
                            SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);
                            SetWindowPos(hWnd, nullptr, 0, 0, (int)DisplayManager::ScreenWidth, (int)DisplayManager::ScreenHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
                        }
                    }
                    else {
                        if (!(exStyle & WS_EX_TRANSPARENT)) {
                            exStyle |= WS_EX_TRANSPARENT;
                            SetWindowLongPtr(hWnd, GWL_EXSTYLE, exStyle);
                            SetWindowPos(hWnd, nullptr, 0, 0, (int)DisplayManager::ScreenWidth, (int)DisplayManager::ScreenHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
                        }
                    }
                }

                // Render ImGui menu
                if (MenuOpen) g_ImGuiMenu.RenderMenu();

                // End ImGui frame
                g_ImGuiMenu.EndFrame();
            }
            catch (const std::exception& e) {
                LOG_CRITICAL("Exception in main game loop: %s", e.what());
                Logger::WriteMiniDump("crash_" + std::to_string(std::time(nullptr)) + ".dmp", nullptr);
                Sleep(1000); // Prevent tight error loop
            }
        }
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("Fatal exception in application: %s", e.what());
        Logger::WriteMiniDump("crash_" + std::to_string(std::time(nullptr)) + ".dmp", nullptr);
        return -1;
    }
    
    LOG_INFO("Application shutting down, cleaning up...");
    Keyboard::StopPolling();
    g_CacheManager.Stop();
    g_ImGuiMenu.Shutdown();

    return 0;
}