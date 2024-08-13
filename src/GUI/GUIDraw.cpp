#include "GUI/GUIDraw.hpp"

void drawUIKeyboardButton(sf::RenderTarget& window, std::string keyString, sf::Vector2f position, float scale, bool centred)
{
    static const sf::Vector2f textOffset(8, 6);

    sf::Vector2f centre(0, 0);
    if (centred) centre = sf::Vector2f(0.5, 0.5);

    // Draw key texture
    TextureManager::drawSubTexture(window, {TextureType::UI, position, 0, sf::Vector2f(scale, scale), centre}, sf::IntRect(0, 0, 16, 16));

    // Draw text for key type
    sf::Vector2f textPos;
    if (centred) textPos = position;
    else textPos = position + textOffset * scale;

    TextDraw::drawText(window, {keyString, textPos, sf::Color(255, 255, 255), static_cast<unsigned int>(7 * scale), {0, 0, 0}, 0, true, true});
}

void drawUIMouseScrollWheelDirection(sf::RenderTarget& window, int direction, sf::Vector2f position, float scale, bool centred)
{
    sf::Vector2f centre(0, 0);
    if (centred) centre = sf::Vector2f(0.5, 0.5);

    // Switch between up and down texture
    sf::IntRect textureRect(32, 0, 16, 16);
    if (direction < 0) textureRect.left = 48;

    TextureManager::drawSubTexture(window, {TextureType::UI, position, 0, sf::Vector2f(scale, scale), centre}, textureRect);
}