#include "GUI/Base/Button.hpp"

Button::Button(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, const std::string& text)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->text = text;

    clicked = false;
    held = false;
    hovered = false;

    if (inputState.activeElement == id)
    {
        held = true;
    }

    if (inputState.leftMouseJustUp && held)
    {
        clicked = true;
    }

    CollisionRect rect(x, y, width, height);
    if (rect.isPointInRect(inputState.mouseX, inputState.mouseY))
    {
        hovered = true;

        if (inputState.leftMouseJustDown)
        {
            held = true;
        }
    }
}

bool Button::isClicked()
{
    return clicked;
}

bool Button::isHeld()
{
    return held;
}

void Button::draw(sf::RenderTarget& window)
{
    // Draw rect
    sf::RectangleShape rect;
    rect.setPosition(x, y);
    rect.setSize(sf::Vector2f(width, height));

    if (clicked || held)
    {
        rect.setFillColor(sf::Color(60, 140, 60));
    }
    else if (hovered)
    {
        rect.setFillColor(sf::Color(0, 240, 0));
    }

    window.draw(rect);

    // Draw text
    TextDrawData textDrawData;
    textDrawData.text = text;
    textDrawData.size = 24;
    textDrawData.colour = sf::Color(0, 0, 0);
    textDrawData.position = sf::Vector2f(x, y) + sf::Vector2f(width, height) / 2.0f;
    textDrawData.centeredX = true;
    textDrawData.centeredY = true;

    TextDraw::drawText(window, textDrawData);
}