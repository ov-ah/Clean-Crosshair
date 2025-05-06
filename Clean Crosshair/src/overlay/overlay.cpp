#include "overlay.h"
#include <../ext/ImGui/imgui.h>
#include <../ext/ImGui/imgui_impl_win32.h>
#include <../ext/ImGui/imgui_impl_dx11.h>
#include <dwmapi.h>
#include "../editor/settings.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dwmapi.lib")

// For system tray menu
#define ID_TRAY_EDITOR     1001
#define ID_TRAY_SETTINGS   1002
#define ID_TRAY_EXIT       1003

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Overlay::Overlay()
    : m_hWnd(nullptr), m_running(false),
    m_pDevice(nullptr), m_pDeviceContext(nullptr), m_pSwapChain(nullptr), m_pRenderTargetView(nullptr),
    m_width(0), m_height(0), m_trayIconAdded(false) {
}

Overlay::~Overlay() {
    shutdown();
}

bool Overlay::initialize() {
    // Create overlay window
    if (!createOverlayWindow()) {
        return false;
    }

    // Initialize Direct3D
    if (!initializeDirect3D()) {
        return false;
    }

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX11_Init(m_pDevice, m_pDeviceContext);

    // Initialize crosshair
    m_crosshair = std::make_shared<Crosshair>();
    m_crosshair->initDefault();

    // Initialize editor window
    m_editorWindow = std::make_unique<EditorWindow>();
    m_editorWindow->initialize();
    m_editorWindow->setCrosshair(m_crosshair);

    // Set callbacks
    m_editorWindow->setCloseCallback([this]() {
        // Do nothing when editor is closed, just hide it
        });

    m_editorWindow->setSaveCallback([this]() {
        // Crosshair updated, no special action needed
        });

    // Add tray icon
    if (!addTrayIcon()) {
        return false;
    }

    // Show editor on startup if not set to start minimized
    if (!Settings::getInstance().startMinimized) {
        m_editorWindow->show(true);
    }

    m_running = true;
    return true;
}

void Overlay::run() {
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    while (m_running) {
        // Process Windows messages
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                m_running = false;
            }
        }

        if (!m_running) {
            break;
        }

        update();
        render();
    }
}

void Overlay::shutdown() {
    // Remove tray icon
    removeTrayIcon();

    // Clean up ImGui
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    // Clean up Direct3D
    cleanupDirect3D();

    // Destroy window
    if (m_hWnd) {
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }

    // Unregister window class
    UnregisterClass(m_wc.lpszClassName, m_wc.hInstance);

    m_running = false;
}

bool Overlay::createOverlayWindow() {
    // Get screen dimensions
    m_width = GetSystemMetrics(SM_CXSCREEN);
    m_height = GetSystemMetrics(SM_CYSCREEN);

    // Register window class
    m_wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Clean Crosshair", nullptr };
    RegisterClassEx(&m_wc);

    // Create window
    m_hWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        m_wc.lpszClassName, L"Clean Crosshair",
        WS_POPUP,
        0, 0, m_width, m_height,
        nullptr, nullptr, m_wc.hInstance, nullptr
    );

    if (!m_hWnd) {
        return false;
    }

    // Store this pointer for use in static window procedure
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Make the window transparent
    SetLayeredWindowAttributes(m_hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    // Make the window click-through
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(m_hWnd, &margins);

    // Make the window click-through
    LONG ex_style = GetWindowLong(m_hWnd, GWL_EXSTYLE);
    ex_style |= WS_EX_LAYERED | WS_EX_TRANSPARENT;
    SetWindowLong(m_hWnd, GWL_EXSTYLE, ex_style);

    // Show the window
    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);

    return true;
}

bool Overlay::initializeDirect3D() {
    // Create the device and swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = m_width;
    sd.BufferDesc.Height = m_height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2,
        D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pDevice, &featureLevel, &m_pDeviceContext
    );

    if (FAILED(hr)) {
        return false;
    }

    // Create render target view
    ID3D11Texture2D* pBackBuffer;
    hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (FAILED(hr)) {
        return false;
    }

    hr = m_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr)) {
        return false;
    }

    return true;
}

void Overlay::cleanupDirect3D() {
    if (m_pRenderTargetView) { m_pRenderTargetView->Release(); m_pRenderTargetView = nullptr; }
    if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
    if (m_pDeviceContext) { m_pDeviceContext->Release(); m_pDeviceContext = nullptr; }
    if (m_pDevice) { m_pDevice->Release(); m_pDevice = nullptr; }
}

void Overlay::update() {
    // Update logic here if needed
}

void Overlay::render() {
    // Start the ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Clear the render