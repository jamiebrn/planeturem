#pragma once

#include <string>
#include <memory>

#include "Graphics/Font.hpp"
#include "Graphics/TextDrawData.hpp"
#include "Graphics/Shader.hpp"

class TextDraw
{
private:
    TextDraw() = delete;

// Public class functions
public:
    // Load font into memory
    static bool loadFont(const std::string& path, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

    static void unloadFont();

    // Draw text using draw data
    static void drawText(pl::RenderTarget& window, const pl::TextDrawData& drawData);

    static pl::Rect<float> getTextSize(const pl::TextDrawData& drawData);

// Private member variables
private:
    // Whether font has been loaded into memory
    static bool loadedFont;

    // Stores font
    static std::unique_ptr<pl::Font> font;

    static std::unique_ptr<pl::Shader> fontShader;

    // Text object used for drawing to screen
    // static sf::Text text;

};
