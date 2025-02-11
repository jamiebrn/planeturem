#pragma once

#include <string>
#include <optional>

#include <SFML/Graphics.hpp>

#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/InputManager.hpp"

#include "GUIElement.hpp"
#include "GUIInputState.hpp"

class Slider : public GUIElement
{
public:
    Slider(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, float minValue, float maxValue,
        float* value, int textSize, std::optional<std::string> label = std::nullopt, int paddingLeft = 0, int paddingRight = 0, int paddingY = 0);

    bool isHeld() const;
    bool hasReleased() const;

    void draw(sf::RenderTarget& window) override;

    sf::IntRect getBoundingBox() const override;

private:
    bool held;
    bool released;

    int x, y, width, height;
    int paddingLeft, paddingRight, paddingY;
    float minValue, maxValue;
    float value;
    std::optional<std::string> label;

};