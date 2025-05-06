#pragma once

#include <string>
#include <memory>
#include <vector>
#include "../common/crosshair.h"

class Settings {
public:
    Settings();
    ~Settings();

    // Load settings from file
    bool load();

    // Save settings to file
    bool save();

    // Application settings
    bool startWithWindows;
    bool startMinimized;
    float crosshairScale;
    std::string lastLoadedPreset;

    // Get singleton instance
    static Settings& getInstance() {
        static Settings instance;
        return instance;
    }

private:
    std::string getSettingsPath() const;
};