#pragma once

#include <string>

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
#include "Core/InputManager.hpp"

#include "GUIInputState.hpp"
#include "GUIElement.hpp"

class TextEnter : public GUIElement
{
public:
    TextEnter(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, int textSize, const std::string& text, std::string* textPtr,
              int paddingX = 0, int paddingY = 0, int maxLength = 9999);

    bool isActive() const;
    bool hasClickedAway() const;

    void draw(pl::RenderTarget& window) override;

    pl::Rect<int> getBoundingBox() const override;

private:
    bool active;
    bool clickedAway;

    int x, y, width, height;
    int paddingX, paddingY;
    std::string text;

    std::string* textPtr;

};