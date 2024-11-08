#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/TextureManager.hpp"

struct ParticleStyle
{
    std::vector<sf::IntRect> textureRects;
    float timePerFrame;
};

class Particle
{
public:
    Particle(sf::Vector2f position, sf::Vector2f velocity, sf::Vector2f acceleration, const ParticleStyle& style);

    void update(float dt);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera) const;

    bool isAlive();

private:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Vector2f acceleration;

    std::vector<sf::IntRect> textureRects;
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

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera) const;

private:
    std::vector<Particle> particles;
};