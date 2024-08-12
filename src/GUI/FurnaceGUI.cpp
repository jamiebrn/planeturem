#include "GUI/FurnaceGUI.hpp"

void FurnaceGUI::draw(sf::RenderTarget& window)
{
    sf::Vector2f screenSize = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

    sf::Vector2f backgroundSize = sf::Vector2f(500, 400) * static_cast<float>(ResolutionHandler::getResolutionIntegerScale());

    sf::RectangleShape background(backgroundSize);
    
    background.setOrigin(backgroundSize / 2.0f);
    background.setPosition(screenSize / 2.0f);
    background.setFillColor(sf::Color(40, 40, 40, 130));

    window.draw(background);

    unsigned int titleFontSize = 48 * ResolutionHandler::getResolutionIntegerScale();
    unsigned int fontSize = 32 * ResolutionHandler::getResolutionIntegerScale();

    TextDraw::drawText(window, {
        "Furnace", screenSize / 2.0f - sf::Vector2f(0, backgroundSize.y / 2.0f) * 4.0f / 5.0f, sf::Color(255, 255, 255), titleFontSize, {0, 0, 0}, 0, true, true
        });

    TextDraw::drawText(window, {"Work in progress", screenSize / 2.0f, sf::Color(255, 255, 255), fontSize, {0, 0, 0}, 0, true, true});
}