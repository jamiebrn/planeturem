#pragma once

#include <string>
#include <optional>

#include <SFML/Graphics.hpp>

#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"

#include "GUIElement.hpp"
#include "GUIInputState.hpp"

struct ButtonStyle
{
    sf::Color colour;
    sf::Color hoveredColour = sf::Color(0, 240, 0);
    sf::Color clickedColour = sf::Color(60, 140, 60);
    sf::Color textColour = sf::Color(0, 0, 0);
    sf::Color hoveredTextColour = sf::Color(0, 0, 0);
    sf::Color clickedTextColour = sf::Color(0, 0, 0);
};

class Button : public GUIElement
{
public:
    Button(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, const std::string& text, std::optional<ButtonStyle> style = std::nullopt);

    bool isClicked();
    bool isHeld();
    bool hasJustReleased();

    void draw(sf::RenderTarget& window) override;

protected:
    bool clicked;
    bool held;
    bool hovered;
    bool justReleased;

    int x, y, width, height;
    std::string text;

    ButtonStyle style;

};