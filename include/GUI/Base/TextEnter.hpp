#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include "Core/TextDraw.hpp"

#include "GUIContext.hpp"
#include "GUIInputState.hpp"
#include "GUIElement.hpp"

class TextEnter : public GUIElement
{
public:
    TextEnter(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, const std::string& text, std::string* textPtr);

    bool isActive();
    bool hasClickedAway();

    void draw(sf::RenderTarget& window) override;

private:
    bool active;
    bool clickedAway;

    int x, y, width, height;
    std::string text;

    std::string* textPtr;

};