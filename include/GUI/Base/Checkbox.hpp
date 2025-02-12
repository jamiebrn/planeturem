#pragma once

#include <string>

#include <SFML/Graphics.hpp>

#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"

#include "GUIElement.hpp"
#include "GUIInputState.hpp"
#include "Button.hpp"

class Checkbox : public Button
{
public:
    Checkbox(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, int textSize, const std::string& label, bool* value,
        int paddingLeft = 0, int paddingRight = 0, int paddingY = 0);

    void draw(sf::RenderTarget& window) override;

    sf::IntRect getBoundingBox() const override;

private:
    bool value;

    int paddingLeft, paddingRight, paddingY;

};