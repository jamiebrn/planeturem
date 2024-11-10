#include "GUI/Base/TextEnter.hpp"

TextEnter::TextEnter(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, const std::string& text, std::string* textPtr)
    : GUIElement(id)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->text = text;
    this->textPtr = textPtr;
    
    active = false;
    clickedAway = false;

    if (inputState.activeElement == id)
    {
        active = true;
    }

    CollisionRect rect(x, y, width, height);
    if (rect.isPointInRect(inputState.mouseX, inputState.mouseY))
    {
        hovered = true;
    }

    if (inputState.leftMouseJustDown)
    {
        if (hovered)
        {
            active = true;
        }
        else if (active)
        {
            clickedAway = true;
        }
    }

    if (active)
    {
        if (inputState.backspaceJustPressed && textPtr->size() > 0)
        {
            textPtr->pop_back(); 
            while (!textPtr->empty() && textPtr->back() == '\0') {
                textPtr->pop_back();
            }
        }
        
        for (unsigned int character : inputState.charEnterBuffer)
        {
            *textPtr += character;
        }
    }
}

bool TextEnter::isActive() const
{
    return active;
}

bool TextEnter::hasClickedAway() const
{
    return clickedAway;
}

void TextEnter::draw(sf::RenderTarget& window)
{
    sf::RectangleShape rect;
    rect.setPosition(x, y);
    rect.setSize(sf::Vector2f(width, height));
    rect.setFillColor(sf::Color(180, 180, 180));

    if (active)
    {
        rect.setFillColor(sf::Color(225, 225, 225));
    }

    window.draw(rect);

    if (textPtr)
    {
        TextDrawData textDrawData;
        textDrawData.position = sf::Vector2f(x + 10, y + height / 2.0f);
        textDrawData.size = 20;
        textDrawData.centeredY = true;
        
        if (textPtr->empty())
        {
            textDrawData.text = text;
        }
        else
        {
            textDrawData.text = *textPtr;
        }

        TextDraw::drawText(window, textDrawData);
    }
}

sf::IntRect TextEnter::getBoundingBox() const
{
    return sf::IntRect(x, y, width, height);
}