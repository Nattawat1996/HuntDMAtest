#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include "ConfigInstance.h"
#include "ConfigUtilities.h"

class ImGuiMenu {
private:
    // D3D11 Device Objects
    ID3D11Device* d3dDevice = nullptr;
    ID3D11DeviceContext* d3dDeviceContext = nullptr;
    IDXGISwapChain* swapChain = nullptr;
    ID3D11RenderTargetView* mainRenderTargetView = nullptr;
    bool initialized = false;

    // Window state
    float lastKeyPressTime = 0.0f;
    static constexpr float KEY_PRESS_DELAY = 0.2f;

    // Helper methods for device management
    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    // Menu rendering methods
    void RenderPlayerESPTab();
    void RenderBossesESPTab();
    void RenderSupplyESPTab();
    void RenderBloodBondsESPTab();
    void RenderTrapESPTab();
    void RenderPOIESPTab();
    void RenderTraitESPTab();
    void RenderOverlayTab();
    void RenderAimbotTab();
    void RenderSettingsTab();
    void RenderHotkeysTab();

    bool RenderFontSizeSlider(const char* label, int& configValue);

    // Theme & Style
    void SetupTheme();

    // Helper methods for UI
    void ColorPickerWithText(const char* label, ImVec4* color);
    void HelpMarker(const char* desc);
    bool HotKey(const char* label, int* key);

public:
    ImGuiMenu() = default;
    ~ImGuiMenu();

    bool Init(HWND hWnd);
    void Shutdown();

    void BeginFrame();
    void RenderMenu();
    void EndFrame();

    void HandleInput();
    void InitializeHotkeys();  // Register keyboard hotkeys for ESP toggles
    bool IsInitialized() const { return initialized; }
};

// Global instance
extern ImGuiMenu g_ImGuiMenu;
extern bool MenuOpen;

// Helper functions
namespace ImGuiUtils {
    bool IsKeyPressed(int vKey);
    float GetTime();
    ImGuiKey VirtualKeyToImGuiKey(int vk);
    int ImGuiKeyToVirtualKey(ImGuiKey key);
    const char* GetKeyName(int vk);
}