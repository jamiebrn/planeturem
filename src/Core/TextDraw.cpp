#include "Core/TextDraw.hpp"

// Initialise member variables, as is static class
bool TextDraw::loadedFont = false;
sf::Font TextDraw::font;
sf::Text TextDraw::text;

// Load font into memory
bool TextDraw::loadFont(std::string path)
{
    // Set loaded font to false by default
    loadedFont = false;

    // If cannot load font data from stream object into font object, return false (unsuccessful load)
    if (!font.loadFromFile(path))
    {
        return false;
    }

    // Set loaded font to true, as font has been loaded
    loadedFont = true;

    // Disable font anti-alisaing
    font.setSmooth(false);
    // Load font object into text object
    text.setFont(font);

    // Return true by default (successful load)
    return true;
}

// Draw text using draw data
void TextDraw::drawText(sf::RenderTarget& window, TextDrawData drawData)
{
    // If font not loaded, do not draw text
    if (!loadedFont)
        return;

    // Set text data from draw data
    text.setString(drawData.text);
    text.setFillColor(drawData.colour);
    text.setCharacterSize(drawData.size);
    text.setOutlineColor(drawData.outlineColour);
    text.setOutlineThickness(drawData.outlineThickness);

    // Clamp to edges of screen if required
    if (drawData.containOnScreenX)
    {
        float width = text.getLocalBounds().width;
        if (drawData.centeredX)
        {
            drawData.position.x = std::min(std::max(
                drawData.position.x, width / 2.0f + drawData.containPaddingLeft), window.getSize().x - width / 2.0f - drawData.containPaddingRight);
        }
        else
        {
            drawData.position.x = std::min(std::max(drawData.position.x, drawData.containPaddingLeft), window.getSize().x - width - drawData.containPaddingRight);
        }
    }
    if (drawData.containOnScreenY)
    {
        float height = text.getLocalBounds().height;
        if (drawData.centeredY)
        {
            drawData.position.y = std::min(std::max(
                drawData.position.y, height / 2.0f + drawData.containPaddingTop), window.getSize().y - height / 2.0f - drawData.containPaddingBottom);
        }
        else
        {
            drawData.position.y = std::min(std::max(drawData.position.y, drawData.containPaddingTop), window.getSize().y - height - drawData.containPaddingBottom);
        }
    }

    // Set position
    text.setPosition(drawData.position);

    // Set text centre to top left by default
    sf::Vector2f textCentre(0, 0);

    // If centre X value, set centre to middle X value
    if (drawData.centeredX)
    {
        textCentre.x = text.getLocalBounds().getSize().x / 2.0f;
    }
    // If centre Y value, set centre to middle Y value
    if (drawData.centeredY)
    {
        textCentre.y = text.getLocalBounds().getSize().y / 2.0f;
        textCentre.y += text.getLocalBounds().getPosition().y;
    }

    // Apply calculted centre as origin
    text.setOrigin(textCentre);

    // Draw text to window
    window.draw(text);
}

sf::FloatRect TextDraw::getTextSize(const TextDrawData& drawData)
{
    text.setString(drawData.text);
    text.setCharacterSize(drawData.size);
    text.setOutlineColor(drawData.outlineColour);
    text.setOutlineThickness(drawData.outlineThickness);

    return text.getLocalBounds();
}