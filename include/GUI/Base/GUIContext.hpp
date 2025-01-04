#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <iostream>

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

    void resetActiveElement();

    const Button& createButton(int x, int y, int width, int height, int textSize, const std::string& text, std::optional<ButtonStyle> style = std::nullopt);

    const Checkbox& createCheckbox(int x, int y, int width, int height, int textSize, const std::string& label, bool* value);

    const Slider& createSlider(int x, int y, int width, int height, float minValue, float maxValue, float* value, int textSize, std::optional<std::string> label = std::nullopt,
        int paddingLeft = 0, int paddingRight = 0, int paddingY = 0);

    const TextEnter& createTextEnter(int x, int y, int width, int height, int textSize, const std::string& text, std::string* textPtr,
        int paddingX = 0, int paddingY = 0, int maxLength = 9999);

    void draw(sf::RenderTarget& window);

    // Used with other control methods, e.g. controller
    void forceElementActivation(ElementID element);

    const GUIElement* getHoveredElement() const;

    const GUIElement* getElementByID(ElementID id) const;

    const inline GUIInputState& getInputState() const {return inputState;}

    const inline bool isElementActive() const {return (inputState.activeElement != std::numeric_limits<uint64_t>::max());}

    const inline int getMaxElementID() const {return std::max(static_cast<int>(elements.size()) - 1, 0);}

private:
    std::vector<std::unique_ptr<GUIElement>> elements;
    
    GUIInputState inputState;

    // bool activeElementRequiresReset = false;

};