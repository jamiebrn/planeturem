#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <iostream>

#include <SDL_events.h>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextDraw.hpp"
#include "Core/Shaders.hpp"

#include "GUIInputState.hpp"
#include "GUIElement.hpp"
#include "Button.hpp"
#include "Checkbox.hpp"
#include "Slider.hpp"
#include "TextEnter.hpp"
#include "ColorWheel.hpp"

class GUIContext
{
public:
    GUIContext() = default;

    void processEvent(const SDL_Event& event);

    void endGUI();

    void resetActiveElement();

    const Button& createButton(int x, int y, int width, int height, int textSize, const std::string& text, std::optional<ButtonStyle> style = std::nullopt);

    const Checkbox& createCheckbox(int x, int y, int width, int height, int textSize, const std::string& label, bool* value,
        int paddingLeft = 0, int paddingRight = 0, int paddingY = 0);

    const Slider& createSlider(int x, int y, int width, int height, float minValue, float maxValue, float* value, int textSize, std::optional<std::string> label = std::nullopt,
        int paddingLeft = 0, int paddingRight = 0, int paddingY = 0);

    const TextEnter& createTextEnter(int x, int y, int width, int height, int textSize, const std::string& text, std::string* textPtr,
        int paddingX = 0, int paddingY = 0, int maxLength = 9999);
    
    const ColorWheel& createColorWheel(int x, int y, int size, pl::Color& currentColor);

    void draw(pl::RenderTarget& window);

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