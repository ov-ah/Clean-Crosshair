#include "crosshairEditor.h"
#include <../ext/ImGui/imgui.h>
#include <algorithm>
#include <cmath>

CrosshairEditor::CrosshairEditor()
    : m_drawColor(255, 255, 255, 255)
    , m_currentTool(Tool::Pencil)
    , m_brushSize(1)
    , m_isDrawing(false)
    , m_startX(0)
    , m_startY(0)
    , m_endX(0)
    , m_endY(0) {
}

CrosshairEditor::~CrosshairEditor() {
}

void CrosshairEditor::initialize(std::shared_ptr<Crosshair> crosshair) {
    m_crosshair = crosshair;
}

void CrosshairEditor::render() {
    if (!m_crosshair) return;

    // Get the grid size
    int gridSize = m_crosshair->getSize();

    // Calculate editor grid size based on available space
    float availWidth = ImGui::GetContentRegionAvail().x - 20.0f;
    float availHeight = ImGui::GetContentRegionAvail().y - 20.0f;
    float cellSize = std::min(availWidth / gridSize, availHeight / gridSize);

    // Center the grid
    float startX = (ImGui::GetContentRegionAvail().x - cellSize * gridSize) / 2.0f;
    float startY = ImGui::GetCursorPosY();

    ImGui::SetCursorPosX(startX);

    // Get mouse position relative to grid
    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 gridPos = ImGui::GetCursorScreenPos();

    int mouseGridX = (int)((mousePos.x - gridPos.x) / cellSize);
    int mouseGridY = (int)((mousePos.y - gridPos.y) / cellSize);

    // Draw grid background
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 gridStart = ImGui::GetCursorScreenPos();

    // Draw grid cells
    for (int y = 0; y < gridSize; y++) {
        for (int x = 0; x < gridSize; x++) {
            Color color = m_crosshair->getPixel(x, y);

            // Calculate cell bounds
            ImVec2 cellMin(gridStart.x + x * cellSize, gridStart.y + y * cellSize);
            ImVec2 cellMax(cellMin.x + cellSize, cellMin.y + cellSize);

            // Draw cell background (checkerboard pattern for transparency)
            bool isCheckered = (x + y) % 2 == 0;
            ImU32 bgColor = isCheckered ? IM_COL32(50, 50, 50, 255) : IM_COL32(30, 30, 30, 255);
            drawList->AddRectFilled(cellMin, cellMax, bgColor);

            // Draw pixel color if not transparent
            if (color.a > 0) {
                drawList->AddRectFilled(cellMin, cellMax, IM_COL32(color.r, color.g, color.b, color.a));
            }

            // Draw cell border
            drawList->AddRect(cellMin, cellMax, IM_COL32(60, 60, 60, 255));
        }
    }

    // Handle mouse input for drawing
    if (ImGui::IsMouseHoveringRect(gridStart, ImVec2(gridStart.x + gridSize * cellSize, gridStart.y + gridSize * cellSize))) {
        // Show cursor position
        ImGui::BeginTooltip();
        ImGui::Text("X: %d, Y: %d", mouseGridX, mouseGridY);
        ImGui::EndTooltip();

        // Handle mouse input
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            m_isDrawing = true;
            m_startX = mouseGridX;
            m_startY = mouseGridY;
            m_endX = mouseGridX;
            m_endY = mouseGridY;

            // Immediate drawing for pencil and eraser
            if (m_currentTool == Tool::Pencil || m_currentTool == Tool::Eraser) {
                if (m_currentTool == Tool::Pencil) {
                    drawPixel(mouseGridX, mouseGridY);
                }
                else {
                    // Eraser sets transparent pixels
                    Color transparent(0, 0, 0, 0);
                    for (int y = mouseGridY - m_brushSize / 2; y <= mouseGridY + m_brushSize / 2; y++) {
                        for (int x = mouseGridX - m_brushSize / 2; x <= mouseGridX + m_brushSize / 2; x++) {
                            if (x >= 0 && x < gridSize && y >= 0 && y < gridSize) {
                                m_crosshair->setPixel(x, y, transparent);
                            }
                        }
                    }
                }
            }
            else if (m_currentTool == Tool::ColorPicker) {
                if (mouseGridX >= 0 && mouseGridX < gridSize && mouseGridY >= 0 && mouseGridY < gridSize) {
                    m_drawColor = m_crosshair->getPixel(mouseGridX, mouseGridY);
                }
            }
        }
        else if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_isDrawing) {
            m_endX = mouseGridX;
            m_endY = mouseGridY;

            // Continuous drawing for pencil and eraser
            if (m_currentTool == Tool::Pencil) {
                drawLine(m_startX, m_startY, m_endX, m_endY);
                m_startX = m_endX;
                m_startY = m_endY;
            }
            else if (m_currentTool == Tool::Eraser) {
                Color transparent(0, 0, 0, 0);
                for (int y = mouseGridY - m_brushSize / 2; y <= mouseGridY + m_brushSize / 2; y++) {
                    for (int x = mouseGridX - m_brushSize / 2; x <= mouseGridX + m_brushSize / 2; x++) {
                        if (x >= 0 && x < gridSize && y >= 0 && y < gridSize) {
                            m_crosshair->setPixel(x, y, transparent);
                        }
                    }
                }
                m_startX = m_endX;
                m_startY = m_endY;
            }
        }
        else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_isDrawing) {
            m_endX = mouseGridX;
            m_endY = mouseGridY;

            // Complete shape drawing
            switch (m_currentTool) {
            case Tool::Line:
                drawLine(m_startX, m_startY, m_endX, m_endY);
                break;
            case Tool::Rectangle:
                drawRectangle(m_startX, m_startY, m_endX, m_endY, false);
                break;
            case Tool::FilledRectangle:
                drawRectangle(m_startX, m_startY, m_endX, m_endY, true);
                break;
            case Tool::Circle:
                drawCircle(m_startX, m_startY, m_endX, m_endY, false);
                break;
            case Tool::FilledCircle:
                drawCircle(m_startX, m_startY, m_endX, m_endY, true);
                break;
            default:
                break;
            }

            m_isDrawing = false;
        }
    }

    // Draw preview of shape being drawn
    if (m_isDrawing) {
        ImVec2 start(gridStart.x + m_startX * cellSize, gridStart.y + m_startY * cellSize);
        ImVec2 end(gridStart.x + m_endX * cellSize + cellSize, gridStart.y + m_endY * cellSize + cellSize);

        // Preview the shape with a dashed line
        if (m_currentTool == Tool::Line) {
            drawList->AddLine(
                ImVec2(start.x + cellSize / 2, start.y + cellSize / 2),
                ImVec2(end.x - cellSize / 2, end.y - cellSize / 2),
                IM_COL32(m_drawColor.r, m_drawColor.g, m_drawColor.b, 128),
                2.0f
            );
        }
        else if (m_currentTool == Tool::Rectangle || m_currentTool == Tool::FilledRectangle) {
            if (m_currentTool == Tool::FilledRectangle) {
                drawList->AddRectFilled(start, end, IM_COL32(m_drawColor.r, m_drawColor.g, m_drawColor.b, 64));
            }
            drawList->AddRect(start, end, IM_COL32(m_drawColor.r, m_drawColor.g, m_drawColor.b, 128), 0.0f, 0, 2.0f);
        }
        else if (m_currentTool == Tool::Circle || m_currentTool == Tool::FilledCircle) {
            float radius = std::sqrt(std::pow(end.x - start.x, 2) + std::pow(end.y - start.y, 2)) / 2;
            ImVec2 center((start.x + end.x) / 2, (start.y + end.y) / 2);
            if (m_currentTool == Tool::FilledCircle) {
                drawList->AddCircleFilled(center, radius, IM_COL32(m_drawColor.r, m_drawColor.g, m_drawColor.b, 64));
            }
            drawList->AddCircle(center, radius, IM_COL32(m_drawColor.r, m_drawColor.g, m_drawColor.b, 128), 0, 2.0f);
        }
    }

    // Move cursor past the grid
    ImGui::SetCursorPosY(startY + gridSize * cellSize + 10.0f);
}

void CrosshairEditor::handleMouseInput(int x, int y, bool leftButton, bool rightButton) {
    // This is handled in the render function now
}

void CrosshairEditor::drawPixel(int x, int y) {
    if (!m_crosshair) return;

    int gridSize = m_crosshair->getSize();
    if (x < 0 || x >= gridSize || y < 0 || y >= gridSize) return;

    // Draw a pixel with the current brush size
    for (int by = y - m_brushSize / 2; by <= y + m_brushSize / 2; by++) {
        for (int bx = x - m_brushSize / 2; bx <= x + m_brushSize / 2; bx++) {
            if (bx >= 0 && bx < gridSize && by >= 0 && by < gridSize) {
                m_crosshair->setPixel(bx, by, m_drawColor);
            }
        }
    }
}

void CrosshairEditor::drawLine(int x1, int y1, int x2, int y2) {
    if (!m_crosshair) return;

    // Bresenham's line algorithm
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true) {
        drawPixel(x1, y1);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

void CrosshairEditor::drawRectangle(int x1, int y1, int x2, int y2, bool filled) {
    if (!m_crosshair) return;

    // Ensure x1,y1 is the top-left and x2,y2 is the bottom-right
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    if (filled) {
        // Fill the rectangle
        for (int y = y1; y <= y2; y++) {
            for (int x = x1; x <= x2; x++) {
                drawPixel(x, y);
            }
        }
    }
    else {
        // Draw the outline
        for (int x = x1; x <= x2; x++) {
            drawPixel(x, y1);
            drawPixel(x, y2);
        }
        for (int y = y1 + 1; y < y2; y++) {
            drawPixel(x1, y);
            drawPixel(x2, y);
        }
    }
}

void CrosshairEditor::drawCircle(int x1, int y1, int x2, int y2, bool filled) {
    if (!m_crosshair) return;

    // Calculate center and radius
    int centerX = (x1 + x2) / 2;
    int centerY = (y1 + y2) / 2;
    int radiusX = std::abs(x2 - x1) / 2;
    int radiusY = std::abs(y2 - y1) / 2;
    int radius = std::max(radiusX, radiusY);

    if (filled) {
        // Fill the circle using distance check
        for (int y = centerY - radius; y <= centerY + radius; y++) {
            for (int x = centerX - radius; x <= centerX + radius; x++) {
                int dx = x - centerX;
                int dy = y - centerY;
                if (dx * dx + dy * dy <= radius * radius) {
                    drawPixel(x, y);
                }
            }
        }
    }
    else {
        // Midpoint circle algorithm for outline
        int x = radius;
        int y = 0;
        int err = 0;

        while (x >= y) {
            drawPixel(centerX + x, centerY + y);
            drawPixel(centerX + y, centerY + x);
            drawPixel(centerX - y, centerY + x);
            drawPixel(centerX - x, centerY + y);
            drawPixel(centerX - x, centerY - y);
            drawPixel(centerX - y, centerY - x);
            drawPixel(centerX + y, centerY - x);
            drawPixel(centerX + x, centerY - y);

            y += 1;
            if (err <= 0) {
                err += 2 * y + 1;
            }
            if (err > 0) {
                x -= 1;
                err -= 2 * x + 1;
            }
        }
    }
}

void CrosshairEditor::clear() {
    if (m_crosshair) {
        m_crosshair->clear();
    }
}

void CrosshairEditor::previewResult() {
    // Show a preview window with the current crosshair
    ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Crosshair Preview", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImVec2 center = ImGui::GetContentRegionAvail();
        center.x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x / 2;
        center.y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y / 2;

        // Draw the crosshair at the center of this window
        renderPreview(center.x, center.y, 2.0f);
    }
    ImGui::End();
}

void CrosshairEditor::renderPreview(float posX, float posY, float scale) {
    if (!m_crosshair