#include "GUI/Base/Checkbox.hpp"

Checkbox::Checkbox(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, int textSize, const std::string& label, bool* value,
    int paddingLeft, int paddingRight, int paddingY)
    : Button(inputState, id, x + paddingLeft, y + paddingY / 2, width - (paddingLeft + paddingRight), height - paddingY, textSize, label)
{
    this->value = *value;
    this->paddingLeft = paddingLeft;
    this->paddingRight = paddingRight;
    this->paddingY = paddingY;

    if (clicked)
    {
        *value = !*value;
    }
}

void Checkbox::draw(pl::RenderTarget& window)
{
    // Draw rect
    pl::Color rectColor(200, 200, 200);
    if (hovered)
    {
        rectColor = pl::Color(245, 245, 245);
    }

    pl::VertexArray rect;
    rect.addQuad(pl::Rect<float>(x, y, width, height), rectColor, pl::Rect<float>());
    
    // Draw inner rect
    if (value || held)
    {
        pl::Color innerRectColor(30, 30, 30);
        
        if (clicked || held)
        {
            innerRectColor = pl::Color(100, 100, 100);
        }
        
        rect.addQuad(pl::Rect<float>(x + width * 0.15f, y + height * 0.15f, width * 0.7f, height * 0.7f),  innerRectColor, pl::Rect<float>());
    }
    
    window.draw(rect, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    // Draw text
    pl::TextDrawData textDrawData;
    textDrawData.text = text;
    textDrawData.size = textSize;
    textDrawData.color = pl::Color(255, 255, 255);
    textDrawData.position = pl::Vector2f(x - paddingLeft + 20.0f, y + height / 2.0f);
    textDrawData.centeredY = true;

    TextDraw::drawText(window, textDrawData);
}

pl::Rect<int> Checkbox::getBoundingBox() const
{
    return pl::Rect<int>(x - paddingLeft, y - paddingY / 2, width + paddingLeft + paddingRight, height + paddingY);
}