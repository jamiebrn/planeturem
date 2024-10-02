#include "GUI/Base/Slider.hpp"

Slider::Slider(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, float minValue, float maxValue, float* value)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->value = value;

    held = false;
    hovered = false;

    if (inputState.activeElement == id)
    {
        held = true;

        // Move slider value as is held
        int sliderXPos = std::max(std::min(inputState.mouseX, x + width), x);
        float sliderProgress = (sliderXPos - x) / static_cast<float>(width);
        *value = sliderProgress * (maxValue - minValue) + minValue;
    }

    CollisionRect sliderRect(x, y, width, height);
    if (sliderRect.isPointInRect(inputState.mouseX, inputState.mouseY))
    {
        hovered = true;
        if (inputState.leftMouseJustDown)
        {
            held = true;
        }
    }
}

bool Slider::isHeld()
{
    return held;
}

void Slider::draw(sf::RenderTarget& window)
{
    sf::RectangleShape sliderRect;
    sliderRect.setPosition(x, y);
    sliderRect.setSize(sf::Vector2f(width, height));
    sliderRect.setFillColor(sf::Color(54, 83, 179));

    if (hovered || held)
    {
        sliderRect.setFillColor(sf::Color(71, 96, 179));
    }

    window.draw(sliderRect);

    if (!value)
        return;

    sf::RectangleShape valueRect;
    int sliderXPos = ((*value - minValue) / (maxValue - minValue)) * width + x;
    valueRect.setPosition(sf::Vector2f(sliderXPos, y + height / 2.0f));
    valueRect.setSize(sf::Vector2f(40, 40));
    valueRect.setFillColor(sf::Color(110, 183, 219));
    valueRect.setOrigin(sf::Vector2f(20, 20));

    if (held)
    {
        valueRect.setFillColor(sf::Color(128, 226, 237));
    }

    window.draw(valueRect);

    TextDrawData textDrawData;
    textDrawData.text = std::to_string(static_cast<int>(*value));
    textDrawData.position = sf::Vector2f(x, y) + sf::Vector2f(width, height) / 2.0f;
    textDrawData.size = 20;
    textDrawData.centeredX = true;
    textDrawData.centeredY = true;
    
    TextDraw::drawText(window, textDrawData);
}