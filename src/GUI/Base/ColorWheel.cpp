#include "GUI/Base/ColorWheel.hpp"

ColorWheel::ColorWheel(const GUIInputState& inputState, ElementID id, int x, int y, int size, float& value, pl::Color& currentColor)
    : GUIElement(id, textSize)
{
    this->x = x;
    this->y = y;
    this->size = size;
    
    active = false;
    clickedAway = false;

    if (inputState.activeElement == id)
    {
        active = true;
    }

    CollisionCircle circle(x, y, size);
    if (circle.isPointColliding(inputState.mouseX, inputState.mouseY) && inputState.leftMousePressed)
    {
        float angle = std::atan2(inputState.mouseY - y, x - inputState.mouseX) + M_PI;
        angle = fmod(360.0f - angle / M_PI * 180.0f, 360.0f);
        float distance = pl::Vector2f(inputState.mouseX - x, inputState.mouseY - y).getLength();

        currentColor = Helper::convertHSVtoRGB(angle, distance / size, value);

        printf("HSV: %f, %f, %f -- RGB: %f, %f, %f\n", angle, distance / size, value, currentColor.r, currentColor.g, currentColor.b);
    }

    this->currentColor = currentColor;
    this->value = value;

    // CollisionRect rect(this->x, this->y, this->width, this->height);
    // if (rect.isPointInRect(inputState.mouseX, inputState.mouseY))
    // {
    //     hovered = true;
    // }

    // if (inputState.leftMouseJustDown)
    // {
    //     if (hovered || (InputManager::isControllerActive() && active))
    //     {
    //         active = true;

    //         // Just activated, open keyboard if required
    //         if (InputManager::isControllerActive())
    //         {
    //             SteamUtils()->ShowFloatingGamepadTextInput(EFloatingGamepadTextInputMode::k_EFloatingGamepadTextInputModeModeSingleLine, x, y, width, height);
    //         }
    //     }
    //     else if (active)
    //     {
    //         clickedAway = true;
    //     }
    // }

    // if (active)
    // {
    //     if (inputState.backspaceJustPressed && textPtr->size() > 0)
    //     {
    //         textPtr->pop_back(); 
    //         while (!textPtr->empty() && textPtr->back() == '\0')
    //         {
    //             textPtr->pop_back();
    //         }
    //     }
        
    //     for (unsigned int character : inputState.charEnterBuffer)
    //     {
    //         if (textPtr->size() >= maxLength)
    //         {
    //             break;
    //         }
    //         *textPtr += character;
    //     }
    // }
}

bool ColorWheel::isActive() const
{
    return active;
}

bool ColorWheel::hasClickedAway() const
{
    return clickedAway;
}

void ColorWheel::draw(pl::RenderTarget& window)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    pl::VertexArray wheel;

    int divisions = DebugOptions::colorWheelDivisions;
    for (int i = 0; i < divisions; i++)
    {
        wheel.addVertex(pl::Vertex(pl::Vector2f((size / intScale / 3.0f) + std::cos(i / static_cast<float>(divisions) * 2 * M_PI) * (size / intScale / 3.0f),
            (size / intScale / 3.0f) + std::sin(i / static_cast<float>(divisions) * 2 * M_PI) * (size / intScale / 3.0f)),
            Helper::convertHSVtoRGB(i / static_cast<float>(divisions) * 360, 1, value)), false);
        wheel.addVertex(pl::Vertex(pl::Vector2f((size / intScale / 3.0f) + std::cos((i + 1) / static_cast<float>(divisions) * 2 * M_PI) * (size / intScale / 3.0f),
            (size / intScale / 3.0f) + std::sin((i + 1) / static_cast<float>(divisions) * 2 * M_PI) * (size / intScale / 3.0f)),
            Helper::convertHSVtoRGB(fmod((i + 1) / static_cast<float>(divisions) * 360, 360), 1, value)), false);
        wheel.addVertex(pl::Vertex(pl::Vector2f((size / intScale / 3.0f), (size / intScale / 3.0f)), pl::Color(255 * value, 255 * value, 255 * value)), false);
    }

    pl::Framebuffer wheelTexture;
    wheelTexture.create(std::ceil((size / intScale) * 2 / 3.0f), std::ceil((size / intScale) * 2 / 3.0f));
    wheelTexture.clear(pl::Color(0, 0, 0, 0));

    wheelTexture.draw(wheel, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    wheel.clear();
    wheel.addQuad(pl::Rect<float>(x - size, y - size, size * 2, size * 2), pl::Color(),
        pl::Rect<float>(0, wheelTexture.getHeight(), wheelTexture.getWidth(), -wheelTexture.getHeight()));
    
    window.draw(wheel, *Shaders::getShader(ShaderType::Default), &wheelTexture.getTexture(), pl::BlendMode::Alpha);

    wheel.clear();

    // Find position of current selector from RGB color
    pl::Color hsvColor = Helper::convertRGBtoHSV(currentColor);
    float angle = hsvColor.r / 360.0f * 2 * M_PI;
    
    pl::Vector2f hsvPosition;
    hsvPosition.x = x + std::cos(angle) * size * hsvColor.g;
    hsvPosition.y = y + std::sin(angle) * size * hsvColor.g;

    wheel.addQuad(pl::Rect<float>(hsvPosition - pl::Vector2f(3, 3) * intScale, pl::Vector2f(6, 6) * intScale), pl::Color(0, 0, 0), pl::Rect<float>());

    wheel.addQuad(pl::Rect<float>(pl::Vector2f(x - size - 20 * intScale, y - 5 * intScale), pl::Vector2f(10, 10) * intScale), currentColor, pl::Rect<float>());

    window.draw(wheel, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);
}

pl::Rect<int> ColorWheel::getBoundingBox() const
{
    return pl::Rect<int>(x - size, y - size, size * 2, size * 2);
}