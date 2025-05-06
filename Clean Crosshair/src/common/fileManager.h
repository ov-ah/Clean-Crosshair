#pragma once

#include <string>
#include <vector>
#include <memory>
#include <windows.h>
#include "../common/crosshair.h"

// Represents a saved crosshair preset
struct CrosshairPreset {
    std::string name;
    std::string data;  // Serialized crosshair data

    CrosshairPreset() {}
    CrosshairPreset(const std::string& name, const std::string& data)
        : name(name), data(data) {
    }
};

class FileManager {
public:
    FileManager();
    ~FileManager();

    // Get application data directory
    std::string getAppDataDirectory() const;

    // Create directories if they don't exist
    bool initializeDirectories();

    // Save a crosshair preset
    bool savePreset(const std::string& name, const Crosshair& crosshair);

    // Load a crosshair preset
    bool loadPreset(const std::string& name, Crosshair& crosshair);

    // Delete a crosshair preset
    bool deletePreset(const std::string& name);

    // Get list of all available presets
    std::vector<std::string> getPresetNames();

    // Load application settings
    bool loadSettings();

    // Save application settings
    bool saveSettings();

private:
    std::string m_appDataPath;
    std::string m_presetsPath;
    std::string m_settingsPath;

    // Get preset file path from name
    std::string getPresetPath(const std::string& name) const;
};