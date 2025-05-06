#include "crosshair.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

// Include ImGui headers here
#include "../ext/ImGui/imgui.h"
#include "../ext/ImGui/imgui_impl_win32.h"
#include "../ext/ImGui/imgui_impl_dx11.h"

uint32_t Color::toImU32() const {
    return IM_COL32(r, g, b, a);
}

Crosshair::Crosshair() : m_size(DEFAULT_SIZE) {
    // Initialize the grid with transparent pixels
    m_pixels.resize(m_size * m_size, Color(0, 0, 0, 0));
}

Crosshair::~Crosshair() {
}

void Crosshair::initDefault() {
    // Clear existing pixels
    clear();

    // Define a simple crosshair (classic plus shape)
    Color white(255, 255, 255, 255);

    int center = m_size / 2;
    int thickness = 2;
    int length = 10;

    // Horizontal line
    for (int x = center - length; x <= center + length; x++) {
        for (int y = center - thickness / 2; y < center + thickness / 2 + (thickness % 2); y++) {
            if (x >= 0 && x < m_size && y >= 0 && y < m_size) {
                setPixel(x, y, white);
            }
        }
    }

    // Vertical line
    for (int y = center - length; y <= center + length; y++) {
        for (int x = center - thickness / 2; x < center + thickness / 2 + (thickness % 2); x++) {
            if (x >= 0 && x < m_size && y >= 0 && y < m_size) {
                setPixel(x, y, white);
            }
        }
    }

    // Add a small gap in the center
    int gap = 2;
    for (int x = center - gap; x <= center + gap; x++) {
        for (int y = center - gap; y <= center + gap; y++) {
            if (x >= 0 && x < m_size && y >= 0 && y < m_size) {
                setPixel(x, y, Color(0, 0, 0, 0)); // Transparent
            }
        }
    }
}

void Crosshair::setPixel(int x, int y, const Color& color) {
    if (x >= 0 && x < m_size && y >= 0 && y < m_size) {
        m_pixels[y * m_size + x] = color;
    }
}

Color Crosshair::getPixel(int x, int y) const {
    if (x >= 0 && x < m_size && y >= 0 && y < m_size) {
        return m_pixels[y * m_size + x];
    }
    return Color(0, 0, 0, 0); // Return transparent if out of bounds
}

void Crosshair::clear() {
    std::fill(m_pixels.begin(), m_pixels.end(), Color(0, 0, 0, 0));
}

void Crosshair::resize(int newSize) {
    if (newSize <= 0) return;

    std::vector<Color> newPixels(newSize * newSize, Color(0, 0, 0, 0));

    // Copy existing pixels to new grid
    int minSize = std::min(m_size, newSize);
    for (int y = 0; y < minSize; y++) {
        for (int x = 0; x < minSize; x++) {
            newPixels[y * newSize + x] = m_pixels[y * m_size + x];
        }
    }

    m_pixels = std::move(newPixels);
    m_size = newSize;
}

void Crosshair::draw(float posX, float posY, float scale) {
    // Get ImGui draw list for rendering
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    // Calculate pixel size based on scale
    float pixelSize = scale;

    // Calculate top-left position to center the crosshair
    float startX = posX - (m_size * pixelSize) / 2.0f;
    float startY = posY - (m_size * pixelSize) / 2.0f;

    // Draw each pixel
    for (int y = 0; y < m_size; y++) {
        for (int x = 0; x < m_size; x++) {
            Color color = getPixel(x, y);

            // Skip fully transparent pixels
            if (color.a == 0) continue;

            // Calculate pixel position
            float pixelX = startX + x * pixelSize;
            float pixelY = startY + y * pixelSize;

            // Draw the pixel as a filled rectangle
            drawList->AddRectFilled(
                ImVec2(pixelX, pixelY),
                ImVec2(pixelX + pixelSize, pixelY + pixelSize),
                color.toImU32()
            );
        }
    }
}

std::string Crosshair::serialize() const {
    std::stringstream ss;

    // Format: size,r,g,b,a,r,g,b,a,...
    ss << m_size;

    for (const auto& pixel : m_pixels) {
        ss << "," << (int)pixel.r
            << "," << (int)pixel.g
            << "," << (int)pixel.b
            << "," << (int)pixel.a;
    }

    return ss.str();
}

bool Crosshair::deserialize(const std::string& data) {
    std::stringstream ss(data);
    std::string token;

    // Get size
    if (!std::getline(ss, token, ',')) return false;

    try {
        int newSize = std::stoi(token);

        if (newSize <= 0) return false;

        std::vector<Color> newPixels;
        newPixels.reserve(newSize * newSize);

        // Read all pixels
        int value;
        for (int i = 0; i < newSize * newSize; i++) {
            Color color;

            // Read r
            if (!std::getline(ss, token, ',')) return false;
            value = std::stoi(token);
            if (value < 0 || value > 255) return false;
            color.r = static_cast<uint8_t>(value);

            // Read g
            if (!std::getline(ss, token, ',')) return false;
            value = std::stoi(token);
            if (value < 0 || value > 255) return false;
            color.g = static_cast<uint8_t>(value);

            // Read b
            if (!std::getline(ss, token, ',')) return false;
            value = std::stoi(token);
            if (value < 0 || value > 255) return false;
            color.b = static_cast<uint8_t>(value);

            // Read a
            if (!std::getline(ss, token, ',')) return false;
            value = std::stoi(token);
            if (value < 0 || value > 255) return false;
            color.a = static_cast<uint8_t>(value);

            newPixels.push_back(color);
        }

        // If we've read all pixels successfully, update the crosshair
        m_pixels = std::move(newPixels);
        m_size = newSize;

        return true;
    }
    catch (const std::exception&) {
        return false;
    }
}