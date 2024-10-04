#pragma once

#include <string>
#include <optional>

#include <SFML/Graphics.hpp>

#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"

#include "GUIElement.hpp"
#include "GUIInputState.hpp"

class Slider : public GUIElement
{
public:
    Slider(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, float minValue, float maxValue,
        float* value, std::optional<std::string> label = std::nullopt);

    bool isHeld();
    bool hasReleased();

    void draw(sf::RenderTarget& window) override;

private:
    bool held;
    bool hovered;
    bool released;

    int x, y, width, height;
    float minValue, maxValue;
    float* value;
    std::optional<std::string> label;

};