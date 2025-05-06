#include "editorWindow.h"
#include <../ext/ImGui/imgui.h>
#include <algorithm>

EditorWindow::EditorWindow()
    : m_visible(false)
    , m_showColorPicker(true)
    , m_showPresets(true)
    , m_showSettings(false)
    , m_currentPreset("Default")
    , m_newPresetName("") {
}

EditorWindow::~EditorWindow() {
}

bool EditorWindow::initialize() {
    // Create editor
    m_editor = std::make_unique<CrosshairEditor>();

    // Create file manager
    m_fileManager = std::make_unique<FileManager>();

    // Initialize directories
    if (!m_fileManager->initializeDirectories()) {
        return false;
    }

    // Load preset list
    refreshPresetList();

    // If presets are empty, create a default
    if (m_presets.empty()) {
        if (m_crosshair) {
            m_crosshair->initDefault();
            savePreset("Default");
            refreshPresetList();
        }
    }

    return true;
}

void EditorWindow::show(bool visible) {
    m_visible = visible;
}

void EditorWindow::toggleVisibility() {
    m_visible = !m_visible;
}

void EditorWindow::setCrosshair(std::shared_ptr<Crosshair> crosshair) {
    m_crosshair = crosshair;

    if (m_editor) {
        m_editor->initialize(crosshair);
    }

    // Try to load the last used preset
    std::string lastPreset = Settings::getInstance().lastLoadedPreset;
    if (!lastPreset.empty()) {
        for (const auto& preset : m_presets) {
            if (preset == lastPreset) {
                loadPreset(lastPreset);
                break;
            }
        }
    }
}

void EditorWindow::render() {
    if (!m_visible || !m_crosshair || !m_editor) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Crosshair Editor", &m_visible)) {
        // Split into two main columns
        ImGui::Columns(2, nullptr, true);

        // Left column: Toolbar and editor
        renderToolbar();

        ImGui::BeginChild("EditorArea", ImVec2(0, 0), true);
        m_editor->render();
        ImGui::EndChild();

        ImGui::NextColumn();

        // Right column: Color picker, presets, settings
        if (m_showColorPicker) {
            renderColorPicker();
        }

        if (m_showPresets) {
            renderPresetManager();
        }

        if (m_showSettings) {
            renderSettings();
        }

        ImGui::Columns(1);
    }
    ImGui::End();

    // If window was closed, call the callback
    if (!m_visible && m_closeCallback) {
        m_closeCallback();
    }
}

void EditorWindow::processInput(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Additional input processing can be added here
}

void EditorWindow::renderToolbar() {
    ImGui::BeginGroup();

    // Tool selection
    if (ImGui::Button("Pencil")) m_editor->setTool(CrosshairEditor::Tool::Pencil);
    ImGui::SameLine();
    if (ImGui::Button("Eraser")) m_editor->setTool(CrosshairEditor::Tool::Eraser);
    ImGui::SameLine();
    if (ImGui::Button("Line")) m_editor->setTool(CrosshairEditor::Tool::Line);
    ImGui::SameLine();
    if (ImGui::Button("Rectangle")) m_editor->setTool(CrosshairEditor::Tool::Rectangle);
    ImGui::SameLine();
    if (ImGui::Button("Filled Rect")) m_editor->setTool(CrosshairEditor::Tool::FilledRectangle);

    if (ImGui::Button("Circle")) m_editor->setTool(CrosshairEditor::Tool::Circle);
    ImGui::SameLine();
    if (ImGui::Button("Filled Circle")) m_editor->setTool(CrosshairEditor::Tool::FilledCircle);
    ImGui::SameLine();
    if (ImGui::Button("Color Picker")) m_editor->setTool(CrosshairEditor::Tool::ColorPicker);
    ImGui::SameLine();
    if (ImGui::Button("Clear")) m_editor->clear();

    // Brush size
    int brushSize = m_editor->getBrushSize();
    if (ImGui::SliderInt("Brush Size", &brushSize, 1, 10)) {
        m_editor->setBrushSize(brushSize);
    }

    // Preview the crosshair
    if (ImGui::Button("Preview")) {
        m_editor->previewResult();
    }

    ImGui::EndGroup();
    ImGui::Separator();
}

void EditorWindow::renderColorPicker() {
    ImGui::BeginGroup();
    ImGui::Text("Color Picker");
    ImGui::Separator();

    Color currentColor = m_editor->getDrawColor();
    float color[4] = {
        currentColor.r / 255.0f,
        currentColor.g / 255.0f,
        currentColor.b / 255.0f,
        currentColor.a / 255.0f
    };

    if (ImGui::ColorPicker4("Current Color", color)) {
        Color newColor(
            static_cast<uint8_t>(color[0] * 255),
            static_cast<uint8_t>(color[1] * 255),
            static_cast<uint8_t>(color[2] * 255),
            static_cast<uint8_t>(color[3] * 255)
        );
        m_editor->setDrawColor(newColor);
    }

    // Common colors
    ImGui::Text("Common Colors");

    if (ImGui::ColorButton("White", ImVec4(1, 1, 1, 1), 0, ImVec2(20, 20))) {
        m_editor->setDrawColor(Color(255, 255, 255, 255));
    }
    ImGui::SameLine();

    if (ImGui::ColorButton("Red", ImVec4(1, 0, 0, 1), 0, ImVec2(20, 20))) {
        m_editor->setDrawColor(Color(255, 0, 0, 255));
    }
    ImGui::SameLine();

    if (ImGui::ColorButton("Green", ImVec4(0, 1, 0, 1), 0, ImVec2(20, 20))) {
        m_editor->setDrawColor(Color(0, 255, 0, 255));
    }
    ImGui::SameLine();

    if (ImGui::ColorButton("Blue", ImVec4(0, 0, 1, 1), 0, ImVec2(20, 20))) {
        m_editor->setDrawColor(Color(0, 0, 255, 255));
    }
    ImGui::SameLine();

    if (ImGui::ColorButton("Yellow", ImVec4(1, 1, 0, 1), 0, ImVec2(20, 20))) {
        m_editor->setDrawColor(Color(255, 255, 0, 255));
    }

    if (ImGui::ColorButton("Cyan", ImVec4(0, 1, 1, 1), 0, ImVec2(20, 20))) {
        m_editor->setDrawColor(Color(0, 255, 255, 255));
    }
    ImGui::SameLine();

    if (ImGui::ColorButton("Magenta", ImVec4(1, 0, 1, 1), 0, ImVec2(20, 20))) {
        m_editor->setDrawColor(Color(255, 0, 255, 255));
    }
    ImGui::SameLine();

    if (ImGui::ColorButton("Orange", ImVec4(1, 0.5, 0, 1), 0, ImVec2(20, 20))) {
        m_editor->setDrawColor(Color(255, 127, 0, 255));
    }
    ImGui::SameLine();

    if (ImGui::ColorButton("Purple", ImVec4(0.5, 0, 0.5, 1), 0, ImVec2(20, 20))) {
        m_editor->setDrawColor(Color(127, 0, 127, 255));
    }
    ImGui::SameLine();

    if (ImGui::ColorButton("Pink", ImVec4(1, 0.5, 0.5, 1), 0, ImVec2(20, 20))) {
        m_editor->setDrawColor(Color(255, 127, 127, 255));
    }

    ImGui::EndGroup();
    ImGui::Separator();
}

void EditorWindow::renderPresetManager() {
    ImGui::BeginGroup();
    ImGui::Text("Presets");
    ImGui::Separator();

    // Save new preset
    ImGui::InputText("Name", &m_newPresetName);
    ImGui::SameLine();
    if (ImGui::Button("Save") && !m_newPresetName.empty()) {
        savePreset(m_newPresetName);
        m_newPresetName.clear();
    }

    ImGui::Text("Available Presets:");
    ImGui::BeginChild("PresetList", ImVec2(0, 150), true);

    for (const auto& preset : m_presets) {
        bool isSelected = (preset == m_currentPreset);
        if (ImGui::Selectable(preset.c_str(), isSelected)) {
            loadPreset(preset);
        }

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Load")) {
                loadPreset(preset);
            }
            if (ImGui::MenuItem("Delete")) {
                deletePreset(preset);
            }
            ImGui::EndPopup();
        }
    }

    ImGui::EndChild();

    ImGui::EndGroup();
    ImGui::Separator();
}

void EditorWindow::renderSettings() {
    ImGui::BeginGroup();
    ImGui::Text("Settings");
    ImGui::Separator();

    Settings& settings = Settings::getInstance();

    ImGui::Checkbox("Start with Windows", &settings.startWithWindows);
    ImGui::Checkbox("Start Minimized", &settings.startMinimized);

    // Crosshair scale slider
    float scale = settings.crosshairScale;
    if (ImGui::SliderFloat("Crosshair Scale", &scale, 0.5f, 5.0f, "%.1f")) {
        settings.crosshairScale = scale;
    }

    if (ImGui::Button("Apply Settings")) {
        applySettings();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Settings")) {
        settings.save();
    }

    ImGui::EndGroup();
}

void EditorWindow::savePreset(const std::string& name) {
    if (!m_crosshair || !m_fileManager) return;

    if (m_fileManager->savePreset(name, *m_crosshair)) {
        m_currentPreset = name;
        refreshPresetList();

        // Update last loaded preset in settings
        Settings::getInstance().lastLoadedPreset = name;
        Settings::getInstance().save();

        // Call the save callback if set
        if (m_saveCallback) {
            m_saveCallback();
        }
    }
}

void EditorWindow::loadPreset(const std::string& name) {
    if (!m_crosshair || !m_fileManager) return;

    if (m_fileManager->loadPreset(name, *m_crosshair)) {
        m_currentPreset = name;

        // Update last loaded preset in settings
        Settings::getInstance().lastLoadedPreset = name;
        Settings::getInstance().save();
    }
}

void EditorWindow::deletePreset(const std::string& name) {
    if (!m_fileManager) return;

    if (m_fileManager->deletePreset(name)) {
        refreshPresetList();

        // If we deleted the current preset, load the first available one
        if (name == m_currentPreset && !m_presets.empty()) {
            loadPreset(m_presets[0]);
        }
    }
}

void EditorWindow::refreshPresetList() {
    if (!m_fileManager) return;

    m_presets = m_fileManager->getPresetNames();
    std::sort(m_presets.begin(), m_presets.end());
}

void EditorWindow::applySettings() {
    Settings& settings = Settings::getInstance();

    // Apply startup with Windows setting
    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (settings.startWithWindows) {
            // Get the executable path
            WCHAR path[MAX_PATH];
            GetModuleFileNameW(NULL, path, MAX_PATH);

            // Add to startup
            RegSetValueExW(hKey, L"CleanCrosshair", 0, REG_SZ, (BYTE*)path, (wcslen(path) + 1) * sizeof(WCHAR));
        }
        else {
            // Remove from startup
            RegDeleteValueW(hKey, L"CleanCrosshair");
        }
        RegCloseKey(hKey);
    }

    // Save settings to file
    settings.save();
}