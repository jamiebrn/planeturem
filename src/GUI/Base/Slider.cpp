#include "GUI/Base/Slider.hpp"

Slider::Slider(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, float minValue, float maxValue,
    float* value, std::optional<std::string> label, int paddingLeft, int paddingRight, int paddingY)
    : GUIElement(id)
{
    this->x = x + paddingLeft;
    this->y = y + paddingY / 2;
    this->width = width - (paddingLeft + paddingRight);
    this->height = height - paddingY;
    this->paddingLeft = paddingLeft;
    this->paddingRight = paddingRight;
    this->paddingY = paddingY;
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->value = value;
    this->label = label;

    held = false;
    hovered = false;
    released = false;

    if (inputState.activeElement == id)
    {
        if (inputState.leftMouseJustUp)
        {
            released = true;
            return;
        }

        held = true;

        // Move slider value as is held
        int sliderXPos = std::max(std::min(inputState.mouseX, this->x + this->width), this->x);
        float sliderProgress = (sliderXPos - this->x) / static_cast<float>(this->width);
        *value = sliderProgress * (maxValue - minValue) + minValue;
    }

    int sliderXPos = ((*value - minValue) / (maxValue - minValue)) * this->width + this->x;

    CollisionRect sliderRect(this->x, this->y, this->width, this->height);
    CollisionRect sliderValueRect(sliderXPos - 20.0f, y + height / 2.0f - 20, 40, 40);
    if (sliderRect.isPointInRect(inputState.mouseX, inputState.mouseY) || sliderValueRect.isPointInRect(inputState.mouseX, inputState.mouseY))
    {
        hovered = true;
        if (inputState.leftMouseJustDown)
        {
            held = true;
        }
    }
}

bool Slider::isHeld() const
{
    return held;
}

bool Slider::hasReleased() const
{
    return released;
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

    if (label.has_value())
    {
        textDrawData.text = label.value();
        textDrawData.position = sf::Vector2f(x, y) + sf::Vector2f(width + 20.0f, height / 2.0f);
        textDrawData.size = 20;
        textDrawData.centeredX = false;
        textDrawData.centeredY = true;
        
        TextDraw::drawText(window, textDrawData);
    }
}

sf::IntRect Slider::getBoundingBox() const
{
    return sf::IntRect(x - paddingLeft, y - paddingY / 2, width + paddingLeft + paddingRight, height + paddingY);
}