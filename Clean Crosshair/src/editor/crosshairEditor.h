#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../common/crosshair.h"

class CrosshairEditor {
public:
    CrosshairEditor();
    ~CrosshairEditor();

    // Initialize editor with a crosshair
    void initialize(std::shared_ptr<Crosshair> crosshair);

    // Render the crosshair editor UI
    void render();

    // Handle mouse input for editing
    void handleMouseInput(int x, int y, bool leftButton, bool rightButton);

    // Set current color for drawing
    void setDrawColor(const Color& color) { m_drawColor = color; }

    // Get current color for drawing
    Color getDrawColor() const { return m_drawColor; }

    // Set current tool
    enum class Tool {
        Pencil,
        Eraser,
        Line,
        Rectangle,
        FilledRectangle,
        Circle,
        FilledCircle,
        ColorPicker
    };

    void setTool(Tool tool) { m_currentTool = tool; }
    Tool getTool() const { return m_currentTool; }

    // Set brush size
    void setBrushSize(int size) { m_brushSize = size; }
    int getBrushSize() const { return m_brushSize; }

    // Clear the crosshair
    void clear();

    // Preview the result
    void previewResult();

private:
    std::shared_ptr<Crosshair> m_crosshair;
    Color m_drawColor;
    Tool m_currentTool;
    int m_brushSize;

    // Tracking for tool operations
    bool m_isDrawing;
    int m_startX;
    int m_startY;
    int m_endX;
    int m_endY;

    // Helper drawing functions
    void drawPixel(int x, int y);
    void drawLine(int x1, int y1, int x2, int y2);
    void drawRectangle(int x1, int y1, int x2, int y2, bool filled);
    void drawCircle(int x1, int y1, int x2, int y2, bool filled);

    // Preview crosshair
    void renderPreview(float posX, float posY, float scale);
};