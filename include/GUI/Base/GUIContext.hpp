#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <optional>

#include <SFML/Graphics.hpp>

#include "GUIInputState.hpp"
#include "GUIElement.hpp"
#include "Button.hpp"
#include "Checkbox.hpp"
#include "Slider.hpp"
#include "TextEnter.hpp"

class GUIContext
{
public:
    GUIContext() = default;

    void processEvent(const sf::Event& event);

    void endGUI();

    bool createButton(int x, int y, int width, int height, const std::string& text);

    bool createCheckbox(int x, int y, int width, int height, const std::string& label, bool* value);

    bool createSlider(int x, int y, int width, int height, float minValue, float maxValue, float* value, std::optional<std::string> label = std::nullopt);

    bool createTextEnter(int x, int y, int width, int height, const std::string& text, std::string* textPtr);

    void draw(sf::RenderTarget& window);

private:
    void resetActiveElement();

private:
    std::vector<std::unique_ptr<GUIElement>> elements;
    
    GUIInputState inputState;

    // bool activeElementRequiresReset = false;

};