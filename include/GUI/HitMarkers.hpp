#pragma once

#include <vector>
#include <string>

#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/DrawData.hpp>
#include <Graphics/TextDrawData.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextDraw.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"

namespace HitMarkers
{

void addHitMarker(pl::Vector2f position, int damageAmount, pl::Color colour = pl::Color(247, 150, 23));

void update(float dt);

void draw(pl::RenderTarget& window, const Camera& camera);

void handleWorldWrap(pl::Vector2f positionDelta);

// Private
namespace 
{
    static constexpr float HIT_MARKER_LIFETIME = 1.5f;
    static constexpr float LIFT_PER_SECOND = 7.0f;
    static constexpr int UNSCALED_FONT_SIZE = 7;

    struct HitMarker
    {
        pl::Vector2f position;
        int damageAmount;
        pl::Color colour;
        float lifetime = 0.0f;

        void update(float dt);
        bool isAlive() const;
        void draw(pl::RenderTarget& window, const Camera& camera) const;
    };

    static std::vector<HitMarker> hitMarkers;
}

}