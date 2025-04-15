#include "Core/TextDraw.hpp"

// Initialise member variables, as is static class
bool TextDraw::loadedFont = false;
std::unique_ptr<pl::Font> TextDraw::font;
std::unique_ptr<pl::Shader> TextDraw::fontShader;
// sf::Text TextDraw::text;

// Load font into memory
bool TextDraw::loadFont(const std::string& path, const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    // Set loaded font to false by default
    loadedFont = false;

    font = std::make_unique<pl::Font>();

    // If cannot load font data from stream object into font object, return false (unsuccessful load)
    if (!font->loadFromFile(path))
    {
        return false;
    }

    fontShader = std::make_unique<pl::Shader>();

    if (!fontShader->load(vertexShaderPath, fragmentShaderPath))
    {
        return false;
    }

    // Set loaded font to true, as font has been loaded
    loadedFont = true;

    // Return true by default (successful load)
    return true;
}

void TextDraw::unloadFont()
{
    font = nullptr;
    fontShader = nullptr;
}

// Draw text using draw data
void TextDraw::drawText(pl::RenderTarget& window, const pl::TextDrawData& drawData)
{
    // If font not loaded, do not draw text
    if (!loadedFont)
    {
        return;
    }

    pl::Rect<float> textSize = font->measureText(drawData);
    pl::VertexArray textBoundingBox;
    textBoundingBox.addQuad(textSize, pl::Color(200, 30, 30), pl::Rect<float>());
    window.draw(textBoundingBox, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    font->draw(window, *fontShader, drawData);
}

pl::Rect<float> TextDraw::getTextSize(const pl::TextDrawData& drawData)
{
    return font->measureText(drawData);
}