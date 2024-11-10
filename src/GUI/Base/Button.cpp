#include "GUI/Base/Button.hpp"

Button::Button(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, const std::string& text, std::optional<ButtonStyle> style)
    : GUIElement(id)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->text = text;

    clicked = false;
    held = false;
    hovered = false;
    justReleased = false;

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

    if (!style.has_value())
    {
        style = ButtonStyle();
    }

    this->style = style.value();
}

bool Button::isClicked() const
{
    return clicked;
}

bool Button::isHeld() const
{
    return held;
}

// bool Button::isHovered() const
// {
//     return hovered;
// }

bool Button::hasJustReleased() const
{
    return justReleased;
}

void Button::draw(sf::RenderTarget& window)
{
    // Draw rect
    sf::RectangleShape rect;
    rect.setPosition(x, y);
    rect.setSize(sf::Vector2f(width, height));
    rect.setFillColor(style.colour);

    // Draw text
    TextDrawData textDrawData;
    textDrawData.text = text;
    textDrawData.size = 24;
    textDrawData.colour = style.textColour;
    textDrawData.position = sf::Vector2f(x, y) + sf::Vector2f(width, height) / 2.0f;
    textDrawData.centeredX = true;
    textDrawData.centeredY = true;
    
    if (clicked || held)
    {
        rect.setFillColor(style.clickedColour);
        textDrawData.colour = style.clickedTextColour;
    }
    else if (hovered)
    {
        rect.setFillColor(style.hoveredColour);
        textDrawData.colour = style.hoveredTextColour;
    }

    window.draw(rect);

    TextDraw::drawText(window, textDrawData);
}

sf::IntRect Button::getBoundingBox() const
{
    return sf::IntRect(x, y, width, height);
}