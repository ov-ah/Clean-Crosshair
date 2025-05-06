#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <windows.h>
#include "../common/crosshair.h"
#include "../common/fileManager.h"
#include "crosshairEditor.h"
#include "settings.h"

class EditorWindow {
public:
    EditorWindow();
    ~EditorWindow();

    // Initialize editor window
    bool initialize();

    // Show or hide the editor window
    void show(bool visible);

    // Check if editor is visible
    bool isVisible() const { return m_visible; }

    // Toggle visibility
    void toggleVisibility();

    // Set crosshair reference
    void setCrosshair(std::shared_ptr<Crosshair> crosshair);

    // Render editor window
    void render();

    // Process input events
    void processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Set callback for when editor is closed
    void setCloseCallback(std::function<void()> callback) { m_closeCallback = callback; }

    // Set callback for when crosshair is saved
    void setSaveCallback(std::function<void()> callback) { m_saveCallback = callback; }

private:
    // UI rendering functions
    void renderToolbar();
    void renderColorPicker();
    void renderPresetManager();
    void renderSettings();

    // Save/load crosshair presets
    void savePreset(const std::string& name);
    void loadPreset(const std::string& name);
    void deletePreset(const std::string& name);
    void refreshPresetList();

    // Apply settings
    void applySettings();

private:
    bool m_visible;
    bool m_showColorPicker;
    bool m_showPresets;
    bool m_showSettings;

    std::shared_ptr<Crosshair> m_crosshair;
    std::unique_ptr<CrosshairEditor> m_editor;
    std::unique_ptr<FileManager> m_fileManager;

    std::vector<std::string> m_presets;
    std::string m_currentPreset;
    std::string m_newPresetName;

    std::function<void()> m_closeCallback;
    std::function<void()> m_saveCallback;
};