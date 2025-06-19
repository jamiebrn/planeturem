#include "GUI/Base/Slider.hpp"

Slider::Slider(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, float minValue, float maxValue,
    float* value, int textSize, std::optional<std::string> label, int paddingLeft, int paddingRight, int paddingY, std::optional<SliderStyle> style)
    : GUIElement(id, textSize)
{
    this->x = x + paddingLeft;
    this->y = y + paddingY / 2;
    this->width = width - (paddingLeft + paddingRight);
    this->height = height - paddingY;
    this->paddingLeft = paddingLeft;
    this->paddingRight = paddingRight;
    this->paddingY = paddingY;
    this->minValue = minValue;
    this->maxValue = maxValue;
    this->value = *value;
    this->label = label;

    if (style.has_value())
    {
        this->style = style.value();
    }

    held = false;
    hovered = false;
    released = false;

    if (inputState.activeElement == id)
    {
        if (InputManager::isControllerActive())
        {
            static constexpr float CONTROLLER_STEP = 0.01f;
            *value = std::clamp(*value + (maxValue - minValue) * CONTROLLER_STEP * InputManager::getActionAxisActivation(InputAction::UI_LEFT, InputAction::UI_RIGHT),
                minValue, maxValue);
        }
        else
        {
            if (inputState.leftMouseJustUp)
            {
                released = true;
                return;
            }

            // Move slider value as is held
            int sliderXPos = std::max(std::min(inputState.mouseX, this->x + this->width - this->height / 2), this->x + this->height / 2);
            float sliderProgress = std::max(std::min((sliderXPos - (this->x + this->height / 2.0f)) / static_cast<float>(this->width - this->height), 1.0f), 0.0f);
            *value = sliderProgress * (maxValue - minValue) + minValue;
        }

        held = true;
    }

    int sliderXPos = ((*value - minValue) / (maxValue - minValue)) * this->width + this->x;

    CollisionRect sliderRect(this->x, this->y, this->width, this->height);
    CollisionRect sliderValueRect(sliderXPos - 20.0f, y + height / 2.0f - 20, 40, 40);
    if (sliderRect.isPointInRect(inputState.mouseX, inputState.mouseY) || sliderValueRect.isPointInRect(inputState.mouseX, inputState.mouseY))
    {
        hovered = true;
        if (inputState.leftMouseJustDown)
        {
            held = true;
        }
    }
}

bool Slider::isHeld() const
{
    return held;
}

bool Slider::hasReleased() const
{
    return released;
}

void Slider::draw(pl::RenderTarget& window)
{
    pl::VertexArray rect;

    pl::Color sliderRectColorLeft = style.sliderColorLeft;
    pl::Color sliderRectColorRight = style.sliderColorRight;
    if (hovered || held)
    {
        sliderRectColorLeft = style.sliderColorLeftHovered;
        sliderRectColorRight = style.sliderColorRightHovered;
    }

    // Add slider quad with potential gradient
    rect.addVertex(pl::Vertex(pl::Vector2f(x, y), sliderRectColorLeft, pl::Vector2f(0, 0)));
    rect.addVertex(pl::Vertex(pl::Vector2f(x, y) + pl::Vector2f(width, 0), sliderRectColorRight, pl::Vector2f(0, 0)));
    rect.addVertex(pl::Vertex(pl::Vector2f(x, y) + pl::Vector2f(0, height), sliderRectColorLeft, pl::Vector2f(0, 0)));
    rect.addVertex(pl::Vertex(pl::Vector2f(x, y) + pl::Vector2f(width, 0), sliderRectColorRight, pl::Vector2f(0, 0)));
    rect.addVertex(pl::Vertex(pl::Vector2f(x, y) + pl::Vector2f(width, height), sliderRectColorRight, pl::Vector2f(0, 0)));
    rect.addVertex(pl::Vertex(pl::Vector2f(x, y) + pl::Vector2f(0, height), sliderRectColorLeft, pl::Vector2f(0, 0)));

    pl::Color valueRectColor = style.valueRectColor;
    if (held)
    {
        valueRectColor = style.valueRectColorHeld;
    }

    int sliderXPos = ((value - minValue) / (maxValue - minValue)) * (width - height) + x + height / 2;
    rect.addQuad(pl::Rect<float>(sliderXPos - height / 2.0f, y, height, height), valueRectColor, pl::Rect<float>());

    window.draw(rect, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    pl::TextDrawData textDrawData;
    textDrawData.text = std::to_string(static_cast<int>(value));
    textDrawData.position = pl::Vector2f(x, y) + pl::Vector2f(width, height) / 2.0f;
    textDrawData.size = textSize;
    textDrawData.centeredX = true;
    textDrawData.centeredY = true;
    textDrawData.color = pl::Color(255, 255, 255);
    
    TextDraw::drawText(window, textDrawData);

    if (label.has_value())
    {
        textDrawData.text = label.value();
        textDrawData.position = pl::Vector2f(x - paddingLeft + 20.0f, y + height / 2.0f);
        textDrawData.centeredX = false;
        textDrawData.centeredY = true;
        
        TextDraw::drawText(window, textDrawData);
    }
}

pl::Rect<int> Slider::getBoundingBox() const
{
    return pl::Rect<int>(x - paddingLeft, y - paddingY / 2, width + paddingLeft + paddingRight, height + paddingY);
}