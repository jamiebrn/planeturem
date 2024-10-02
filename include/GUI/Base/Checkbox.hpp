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
    Checkbox(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, const std::string& label, bool* value);

    void draw(sf::RenderTarget& window) override;

private:
    bool* value;

};