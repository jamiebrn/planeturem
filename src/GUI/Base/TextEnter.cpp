#include "GUI/Base/TextEnter.hpp"

TextEnter::TextEnter(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, int textSize, const std::string& text, std::string* textPtr,
                     int paddingX, int paddingY, int maxLength)
    : GUIElement(id, textSize)
{
    this->x = x + paddingX / 2;
    this->y = y + paddingY / 2;
    this->width = width - paddingX;
    this->height = height - paddingY;
    this->paddingX = paddingX;
    this->paddingY = paddingY;
    this->text = text;
    this->textPtr = textPtr;
    
    active = false;
    clickedAway = false;

    if (inputState.activeElement == id)
    {
        active = true;
    }

    CollisionRect rect(this->x, this->y, this->width, this->height);
    if (rect.isPointInRect(inputState.mouseX, inputState.mouseY))
    {
        hovered = true;
    }

    if (inputState.leftMouseJustDown)
    {
        if (hovered || (InputManager::isControllerActive() && active))
        {
            active = true;

            // Just activated, open keyboard if required
            if (InputManager::isControllerActive())
            {
                SteamUtils()->ShowFloatingGamepadTextInput(EFloatingGamepadTextInputMode::k_EFloatingGamepadTextInputModeModeSingleLine, x, y, width, height);
            }
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
            if (textPtr->size() >= maxLength)
            {
                break;
            }
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
        textDrawData.size = textSize;
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
    return sf::IntRect(x - paddingX / 2, y - paddingY / 2, width + paddingX, height + paddingY);
}