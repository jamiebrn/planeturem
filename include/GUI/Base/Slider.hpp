#pragma once

#include <string>

#include <SFML/Graphics.hpp>

#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"

#include "GUIElement.hpp"
#include "GUIInputState.hpp"

class Slider : public GUIElement
{
public:
    Slider(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, float minValue, float maxValue, float* value);

    bool isHeld();

    void draw(sf::RenderTarget& window) override;

private:
    bool held;
    bool hovered;

    int x, y, width, height;
    float minValue, maxValue;
    float* value;

};