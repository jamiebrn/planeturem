#include "GUI/Base/Button.hpp"

Button::Button(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, int textSize, const std::string& text, std::optional<ButtonStyle> style)
    : GUIElement(id, textSize)
{
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->text = text;

    clicked = false;
    held = false;
    hovered = false;
    justReleased = false;

    if (inputState.activeElement == id)
    {
        held = true;
    }

    if (inputState.leftMouseJustUp && held)
    {
        clicked = true;

        // Play click sound
        Sounds::playSound(SoundType::UIClick0, 30.0f);
    }

    CollisionRect rect(x, y, width, height);
    if (rect.isPointInRect(inputState.mouseX, inputState.mouseY))
    {
        hovered = true;

        if (inputState.leftMouseJustDown)
        {
            held = true;
        }
    }

    if (!style.has_value())
    {
        style = ButtonStyle();
    }

    this->style = style.value();
}

bool Button::isClicked() const
{
    return clicked;
}

bool Button::isHeld() const
{
    return held;
}

// bool Button::isHovered() const
// {
//     return hovered;
// }

bool Button::hasJustReleased() const
{
    return justReleased;
}

void Button::draw(pl::RenderTarget& window)
{
    // Draw text
    pl::TextDrawData textDrawData;
    textDrawData.text = text;
    textDrawData.size = textSize;
    textDrawData.color = style.textColor;
    textDrawData.position = pl::Vector2f(x, y) + pl::Vector2f(width, height) / 2.0f;
    textDrawData.centeredX = true;
    textDrawData.centeredY = true;

    pl::Color rectColor;

    if (clicked || held)
    {
        rectColor = style.clickedColor;
        textDrawData.color = style.clickedTextColor;
    }
    else if (hovered)
    {
        rectColor = style.hoveredColor;
        textDrawData.color = style.hoveredTextColor;
    }

    // Draw rect
    pl::VertexArray rect;
    rect.addQuad(pl::Rect<float>(x, y, width, height), rectColor, pl::Rect<float>());
    window.draw(rect, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    TextDraw::drawText(window, textDrawData);
}

pl::Rect<int> Button::getBoundingBox() const
{
    return pl::Rect<int>(x, y, width, height);
}