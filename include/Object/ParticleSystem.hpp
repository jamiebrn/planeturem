#pragma once

#include <vector>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/DrawData.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"
#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"

struct ParticleStyle
{
    std::vector<pl::Rect<int>> textureRects;
    float timePerFrame;
};

class Particle
{
public:
    Particle(pl::Vector2f position, pl::Vector2f velocity, pl::Vector2f acceleration, const ParticleStyle& style);

    void update(float dt);

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, int worldSize) const;

    bool isAlive();

    // void handleWorldWrap(pl::Vector2f positionDelta);

private:
    pl::Vector2f position;
    pl::Vector2f velocity;
    pl::Vector2f acceleration;

    std::vector<pl::Rect<int>> textureRects;
    int currentFrame;
    float frameTimer;
    float timePerFrame;
};

class ParticleSystem
{
public:
    ParticleSystem() = default;

    void addParticle(const Particle& particle);

    void update(float dt);

    void draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, int worldSize) const;

    // void handleWorldWrap(pl::Vector2f positionDelta);

    void clear();

private:
    std::vector<Particle> particles;
};