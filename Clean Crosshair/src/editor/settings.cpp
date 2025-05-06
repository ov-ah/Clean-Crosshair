#include "settings.h"
#include <fstream>
#include <shlobj.h>
#include <filesystem>

Settings::Settings()
    : startWithWindows(false)
    , startMinimized(false)
    , crosshairScale(1.0f)
    , lastLoadedPreset("Default") {
    // Load settings from file
    load();
}

Settings::~Settings() {
    // Save settings when the application exits
    save();
}

std::string Settings::getSettingsPath() const {
    WCHAR path[MAX_PATH];

    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        char pathA[MAX_PATH];
        wcstombs(pathA, path, sizeof(pathA));
        std::string appDataPath = std::string(pathA) + "\\CleanCrosshair";

        // Create directory if it doesn't exist
        if (!std::filesystem::exists(appDataPath)) {
            std::filesystem::create_directory(appDataPath);
        }

        return appDataPath + "\\settings.cfg";
    }

    return "";
}

bool Settings::load() {
    std::string path = getSettingsPath();
    std::ifstream file(path);

    if (!file.is_open()) {
        // If file doesn't exist, use default settings
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t delimPos = line.find('=');
        if (delimPos != std::string::npos) {
            std::string key = line.substr(0, delimPos);
            std::string value = line.substr(delimPos + 1);

            if (key == "StartWithWindows") {
                startWithWindows = (value == "true");
            }
            else if (key == "StartMinimized") {
                startMinimized = (value == "true");
            }
            else if (key == "CrosshairScale") {
                try {
                    crosshairScale = std::stof(value);
                }
                catch (...) {
                    crosshairScale = 1.0f;
                }
            }
            else if (key == "LastLoadedPreset") {
                lastLoadedPreset = value;
            }
        }
    }

    file.close();
    return true;
}

bool Settings::save() {
    std::string path = getSettingsPath();
    std::ofstream file(path);

    if (!file.is_open()) {
        return false;
    }

    file << "StartWithWindows=" << (startWithWindows ? "true" : "false") << std::endl;
    file << "StartMinimized=" << (startMinimized ? "true" : "false") << std::endl;
    file << "CrosshairScale=" << crosshairScale << std::endl;
    file << "LastLoadedPreset=" << lastLoadedPreset << std::endl;

    file.close();
    return true;
}