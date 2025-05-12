#pragma once

#include <string>
#define _USE_MATH_DEFINES
#include <cmath>

#include <extlib/steam/steam_api.h>

#include <Graphics/VertexArray.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Shaders.hpp"
#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/CollisionCircle.hpp"
#include "Core/InputManager.hpp"
#include "Core/Helper.hpp"

#include "GUIInputState.hpp"
#include "GUIElement.hpp"

class ColorWheel : public GUIElement
{
public:
    ColorWheel(const GUIInputState& inputState, ElementID id, int x, int y, int size, float& value, pl::Color& currentColor);

    bool isActive() const;
    bool hasClickedAway() const;

    void draw(pl::RenderTarget& window) override;

    pl::Rect<int> getBoundingBox() const override;

private:
    bool active;
    bool clickedAway;

    int x, y, size;
    pl::Color currentColor;
    float value;

};