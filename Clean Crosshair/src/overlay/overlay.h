#pragma once

#include <windows.h>
#include <memory>
#include <string>
#include <d3d11.h>
#include "../common/crosshair.h"
#include "../editor/editorWindow.h"

// Define custom message for system tray
#define WM_TRAYICON (WM_USER + 1)

class Overlay {
public:
    Overlay();
    ~Overlay();

    // Initialize overlay window and Direct3D
    bool initialize();

    // Run the main message loop
    void run();

    // Terminate the overlay
    void shutdown();

    // Check if overlay is running
    bool isRunning() const { return m_running; }

    // Get crosshair instance
    Crosshair& getCrosshair() { return *m_crosshair; }

private:
    // Window procedure
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Render frame
    void render();

    // Update frame
    void update();

    // Create overlay window
    bool createOverlayWindow();

    // Initialize Direct3D
    bool initializeDirect3D();

    // Clean up Direct3D resources
    void cleanupDirect3D();

    // Add system tray icon
    bool addTrayIcon();

    // Remove system tray icon
    void removeTrayIcon();

    // Show system tray context menu
    void showTrayContextMenu();

    // Toggle editor window visibility
    void toggleEditor();

private:
    HWND m_hWnd;
    WNDCLASSEX m_wc;
    bool m_running;

    // Direct3D resources
    ID3D11Device* m_pDevice;
    ID3D11DeviceContext* m_pDeviceContext;
    IDXGISwapChain* m_pSwapChain;
    ID3D11RenderTargetView* m_pRenderTargetView;

    // Crosshair data
    std::shared_ptr<Crosshair> m_crosshair;

    // Editor window
    std::unique_ptr<EditorWindow> m_editorWindow;

    // Window dimensions
    int m_width;
    int m_height;

    // System tray icon data
    NOTIFYICONDATA m_trayIconData;
    bool m_trayIconAdded;
};