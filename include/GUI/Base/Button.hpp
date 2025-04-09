#pragma once

#include <string>
#include <optional>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Graphics/VertexArray.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Shaders.hpp"
#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/Sounds.hpp"

#include "GUIElement.hpp"
#include "GUIInputState.hpp"

struct ButtonStyle
{
    pl::Color colour = pl::Color(255, 255, 255);
    pl::Color hoveredColour = pl::Color(0, 240, 0);
    pl::Color clickedColour = pl::Color(60, 140, 60);
    pl::Color textColour = pl::Color(0, 0, 0);
    pl::Color hoveredTextColour = pl::Color(0, 0, 0);
    pl::Color clickedTextColour = pl::Color(0, 0, 0);
};

class Button : public GUIElement
{
public:
    Button(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, int textSize, const std::string& text,
            std::optional<ButtonStyle> style = std::nullopt);

    bool isClicked() const;
    bool isHeld() const;
    // virtual bool isHovered() const override;
    bool hasJustReleased() const;

    void draw(pl::RenderTarget& window) override;

    pl::Rect<int> getBoundingBox() const override;

protected:
    bool clicked;
    bool held;
    // bool hovered;
    bool justReleased;

    int x, y, width, height;
    std::string text;

    ButtonStyle style;

};