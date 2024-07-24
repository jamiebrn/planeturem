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
void TextDraw::drawText(sf::RenderWindow& window, TextDrawData drawData)
{
    // If font not loaded, do not draw text
    if (!loadedFont)
        return;
    
    // Set text data from draw data
    text.setString(drawData.text);
    text.setPosition(drawData.position);
    text.setFillColor(drawData.colour);
    text.setCharacterSize(drawData.size);
    text.setOutlineColor(drawData.outlineColour);
    text.setOutlineThickness(drawData.outlineThickness);

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