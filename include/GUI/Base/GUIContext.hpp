#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>

#include <SFML/Graphics.hpp>

#include "GUIInputState.hpp"
#include "GUIElement.hpp"
#include "Button.hpp"
#include "Checkbox.hpp"
#include "Slider.hpp"

class GUIContext
{
public:
    GUIContext() = default;

    void processEvent(const sf::Event& event);

    void beginGUI();

    bool createButton(int x, int y, int width, int height, const std::string& text);

    bool createCheckbox(int x, int y, int width, int height, const std::string& label, bool* value);

    bool createSlider(int x, int y, int width, int height, float minValue, float maxValue, float* value);

    void draw(sf::RenderTarget& window);

private:
    void resetActiveElement();

private:
    std::vector<std::unique_ptr<GUIElement>> elements;
    
    GUIInputState inputState;

    bool activeElementRequiresReset = true;

};