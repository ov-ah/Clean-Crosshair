#include "fileManager.h"
#include <shlobj.h>
#include <fstream>
#include <filesystem>
#include <direct.h>

FileManager::FileManager() {
    m_appDataPath = getAppDataDirectory() + "\\CleanCrosshair";
    m_presetsPath = m_appDataPath + "\\Presets";
    m_settingsPath = m_appDataPath + "\\settings.cfg";

    initializeDirectories();
}

FileManager::~FileManager() {
}

std::string FileManager::getAppDataDirectory() const {
    WCHAR path[MAX_PATH];

    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        char pathA[MAX_PATH];
        wcstombs(pathA, path, sizeof(pathA));
        return std::string(pathA);
    }

    return "";
}

bool FileManager::initializeDirectories() {
    // Create app data directory if it doesn't exist
    if (!std::filesystem::exists(m_appDataPath)) {
        if (!std::filesystem::create_directory(m_appDataPath)) {
            return false;
        }
    }

    // Create presets directory if it doesn't exist
    if (!std::filesystem::exists(m_presetsPath)) {
        if (!std::filesystem::create_directory(m_presetsPath)) {
            return false;
        }
    }

    return true;
}

std::string FileManager::getPresetPath(const std::string& name) const {
    // Sanitize name to ensure it's a valid filename
    std::string safeName = name;
    for (char& c : safeName) {
        if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            c = '_';
        }
    }

    return m_presetsPath + "\\" + safeName + ".crosshair";
}

bool FileManager::savePreset(const std::string& name, const Crosshair& crosshair) {
    std::string filePath = getPresetPath(name);
    std::ofstream file(filePath);

    if (!file.is_open()) {
        return false;
    }

    // Write serialized crosshair data
    file << crosshair.serialize();
    file.close();

    return true;
}

bool FileManager::loadPreset(const std::string& name, Crosshair& crosshair) {
    std::string filePath = getPresetPath(name);
    std::ifstream file(filePath);

    if (!file.is_open()) {
        return false;
    }

    // Read serialized crosshair data
    std::string data;
    std::getline(file, data);
    file.close();

    return crosshair.deserialize(data);
}

bool FileManager::deletePreset(const std::string& name) {
    std::string filePath = getPresetPath(name);
    return std::filesystem::remove(filePath);
}

std::vector<std::string> FileManager::getPresetNames() {
    std::vector<std::string> presets;

    for (const auto& entry : std::filesystem::directory_iterator(m_presetsPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".crosshair") {
            std::string filename = entry.path().filename().string();
            // Remove .crosshair extension
            presets.push_back(filename.substr(0, filename.length() - 10));
        }
    }

    return presets;
}

bool FileManager::loadSettings() {
    // This will be implemented when we add settings
    return true;
}

bool FileManager::saveSettings() {
    // This will be implemented when we add settings
    return true;
}