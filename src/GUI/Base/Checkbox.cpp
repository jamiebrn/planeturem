#include "GUI/Base/Checkbox.hpp"

Checkbox::Checkbox(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, int textSize, const std::string& label, bool* value)
    : Button(inputState, id, x, y, width, height, textSize, label)
{
    this->value = value;

    if (clicked)
    {
        *value = !*value;
    }
}

void Checkbox::draw(sf::RenderTarget& window)
{
    // Draw rect
    sf::RectangleShape rect;
    rect.setPosition(x, y);
    rect.setSize(sf::Vector2f(width, height));
    rect.setFillColor(sf::Color(200, 200, 200));

    if (hovered)
    {
        rect.setFillColor(sf::Color(245, 245, 245));
    }

    window.draw(rect);

    // Draw inner rect
    if (value)
    {
        if (*value || held)
        {
            sf::RectangleShape innerRect;
            innerRect.setPosition(x + width * 0.15f, y + height * 0.15f);
            innerRect.setSize(sf::Vector2f(width * 0.7f, height * 0.7f));
            innerRect.setFillColor(sf::Color(30, 30, 30));

            if (clicked || held)
            {
                innerRect.setFillColor(sf::Color(100, 100, 100));
            }

            window.draw(innerRect);
        }
    }

    // Draw text
    TextDrawData textDrawData;
    textDrawData.text = text;
    textDrawData.size = textSize;
    textDrawData.colour = sf::Color(0, 0, 0);
    textDrawData.position = sf::Vector2f(x, y) + sf::Vector2f(width * 1.2f, height / 2.0f);
    textDrawData.centeredY = true;

    TextDraw::drawText(window, textDrawData);
}