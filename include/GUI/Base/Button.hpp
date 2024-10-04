#pragma once

#include <string>

#include <SFML/Graphics.hpp>

#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"

#include "GUIElement.hpp"
#include "GUIInputState.hpp"

class Button : public GUIElement
{
public:
    Button(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, const std::string& text);

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

};