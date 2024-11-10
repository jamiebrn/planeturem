#pragma once

#include <SFML/Graphics.hpp>
#include <string>

#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"

#include "GUIInputState.hpp"
#include "GUIElement.hpp"

class TextEnter : public GUIElement
{
public:
    TextEnter(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, const std::string& text, std::string* textPtr);

    bool isActive() const;
    bool hasClickedAway() const;

    void draw(sf::RenderTarget& window) override;

    sf::IntRect getBoundingBox() const override;

private:
    bool active;
    bool clickedAway;

    int x, y, width, height;
    std::string text;

    std::string* textPtr;

};