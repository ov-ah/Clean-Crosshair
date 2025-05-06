#include <windows.h>
#include <memory>
#include "overlay/overlay.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Create the overlay
    std::unique_ptr<Overlay> overlay = std::make_unique<Overlay>();

    // Initialize overlay
    if (!overlay->initialize()) {
        MessageBox(NULL, L"Failed to initialize the overlay!", L"Error", MB_ICONERROR);
        return 1;
    }

    // Run overlay main loop
    overlay->run();

    // Clean up is handled by the Overlay destructor
    return 0;
}