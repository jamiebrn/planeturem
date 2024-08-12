#pragma once

#include <SFML/Graphics.hpp>
#include <string>

// Struct containing all data required when drawing text
struct TextDrawData
{
    std::string text;
    sf::Vector2f position;
    sf::Color colour;
    unsigned int size;

    sf::Color outlineColour = sf::Color(0, 0, 0);
    float outlineThickness = 0;

    bool centeredX = false;
    bool centeredY = false;
};

// Declare text renderer class
class TextDraw
{

// Delete constructor, as is static class
private:
    TextDraw() = delete;

// Public class functions
public:
    // Load font into memory
    static bool loadFont(std::string path);

    // Draw text using draw data
    static void drawText(sf::RenderTarget& window, TextDrawData drawData);

// Private member variables
private:
    // Whether font has been loaded into memory
    static bool loadedFont;

    // Stores font
    static sf::Font font;

    // Text object used for drawing to screen
    static sf::Text text;

};
