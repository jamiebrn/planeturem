#pragma once

#include <string>

#include <Graphics/VertexArray.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Shaders.hpp"
#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"

#include "GUIElement.hpp"
#include "GUIInputState.hpp"
#include "Button.hpp"

class Checkbox : public Button
{
public:
    Checkbox(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, int textSize, const std::string& label, bool* value,
        int paddingLeft = 0, int paddingRight = 0, int paddingY = 0);

    void draw(pl::RenderTarget& window) override;

    pl::Rect<int> getBoundingBox() const override;

private:
    bool value;

    int paddingLeft, paddingRight, paddingY;

};