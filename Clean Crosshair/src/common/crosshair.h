#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cstdint>

// RGBA color representation
struct Color {
    uint8_t r, g, b, a;

    Color() : r(0), g(0), b(0), a(0) {}
    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

    // Convert color to uint32_t representation (for ImGui)
    uint32_t toImU32() const;
};

class Crosshair {
public:
    static const int DEFAULT_SIZE = 64;  // Default grid size (64x64)

    Crosshair();
    ~Crosshair();

    // Initialize with default crosshair
    void initDefault();

    // Set pixel at position
    void setPixel(int x, int y, const Color& color);

    // Get pixel at position
    Color getPixel(int x, int y) const;

    // Clear all pixels
    void clear();

    // Resize grid (preserves content where possible)
    void resize(int newSize);

    // Get current size
    int getSize() const { return m_size; }

    // Draw crosshair at specified position
    void draw(float posX, float posY, float scale = 1.0f);

    // Serialize to string (for saving)
    std::string serialize() const;

    // Deserialize from string (for loading)
    bool deserialize(const std::string& data);

private:
    std::vector<Color> m_pixels;
    int m_size;
};