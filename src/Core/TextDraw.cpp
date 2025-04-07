#include "Core/TextDraw.hpp"

// Initialise member variables, as is static class
bool TextDraw::loadedFont = false;
pl::Font TextDraw::font;
pl::Shader TextDraw::fontShader;
// sf::Text TextDraw::text;

// Load font into memory
bool TextDraw::loadFont(const std::string& path, const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    // Set loaded font to false by default
    loadedFont = false;

    // If cannot load font data from stream object into font object, return false (unsuccessful load)
    if (!font.loadFromFile(path))
    {
        return false;
    }

    if (!fontShader.load(vertexShaderPath, fragmentShaderPath))
    {
        return false;
    }

    // Set loaded font to true, as font has been loaded
    loadedFont = true;

    // Disable font anti-alisaing
    // font.setSmooth(false);
    // Load font object into text object
    // text.setFont(font);

    // Return true by default (successful load)
    return true;
}

// Draw text using draw data
void TextDraw::drawText(pl::RenderTarget& window, const pl::TextDrawData& drawData)
{
    // If font not loaded, do not draw text
    if (!loadedFont)
    {
        return;
    }

    font.draw(window, fontShader, drawData);

    // Set text data from draw data
    // text.setString(drawData.text);
    // text.setFillColor(drawData.colour);
    // text.setCharacterSize(drawData.size);
    // text.setOutlineColor(drawData.outlineColour);
    // text.setOutlineThickness(drawData.outlineThickness);

    // sf::Vector2f drawPos = drawData.position;

    // Clamp to edges of screen if required
    // if (drawData.containOnScreenX)
    // {
    //     float width = text.getLocalBounds().width;
    //     if (drawData.centeredX)
    //     {
    //         drawPos.x = std::min(std::max(
    //             drawPos.x, width / 2.0f + drawData.containPaddingLeft), window.getSize().x - width / 2.0f - drawData.containPaddingRight);
    //     }
    //     else
    //     {
    //         drawPos.x = std::min(std::max(drawPos.x, drawData.containPaddingLeft), window.getSize().x - width - drawData.containPaddingRight);
    //     }
    // }
    // if (drawData.containOnScreenY)
    // {
    //     float height = text.getLocalBounds().height;
    //     if (drawData.centeredY)
    //     {
    //         drawPos.y = std::min(std::max(
    //             drawPos.y, height / 2.0f + drawData.containPaddingTop), window.getSize().y - height / 2.0f - drawData.containPaddingBottom);
    //     }
    //     else
    //     {
    //         drawPos.y = std::min(std::max(drawPos.y, drawData.containPaddingTop), window.getSize().y - height - drawData.containPaddingBottom);
    //     }
    // }

    // // Set position
    // text.setPosition(sf::Vector2f(static_cast<int>(drawPos.x), static_cast<int>(drawPos.y)));

    // // Set text centre to top left by default
    // sf::Vector2f textCentre(0, 0);

    // // If centre X value, set centre to middle X value
    // if (drawData.centeredX)
    // {
    //     textCentre.x = static_cast<int>(text.getLocalBounds().getSize().x / 2.0f);
    // }
    // // If centre Y value, set centre to middle Y value
    // if (drawData.centeredY)
    // {
    //     textCentre.y = static_cast<int>(text.getLocalBounds().getSize().y / 2.0f);
    //     textCentre.y += text.getLocalBounds().getPosition().y;
    // }

    // // Apply calculted centre as origin
    // text.setOrigin(textCentre);

    // Draw text to window
    // window.draw(text);
}

pl::Rect<float> TextDraw::getTextSize(const pl::TextDrawData& drawData)
{
    // text.setString(drawData.text);
    // text.setCharacterSize(drawData.size);
    // text.setOutlineColor(drawData.outlineColour);
    // text.setOutlineThickness(drawData.outlineThickness);

    // return text.getLocalBounds();

    return font.measureText(drawData);
}