#pragma once

#include <vector>
#include <string>

#include <SFML/Graphics.hpp>

#include "Core/TextDraw.hpp"
#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"

namespace HitMarkers
{

void addHitMarker(sf::Vector2f position, int damageAmount, sf::Color colour = sf::Color(247, 150, 23));

void update(float dt);

void draw(sf::RenderTarget& window, const Camera& camera);

void handleWorldWrap(sf::Vector2f positionDelta);

// Private
namespace 
{
    static constexpr float HIT_MARKER_LIFETIME = 1.5f;
    static constexpr float LIFT_PER_SECOND = 7.0f;
    static constexpr int UNSCALED_FONT_SIZE = 7;

    struct HitMarker
    {
        sf::Vector2f position;
        int damageAmount;
        sf::Color colour;
        float lifetime = 0.0f;

        void update(float dt);
        bool isAlive() const;
        void draw(sf::RenderTarget& window, const Camera& camera) const;
    };

    static std::vector<HitMarker> hitMarkers;
}

}