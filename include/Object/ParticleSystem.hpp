#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "Core/Camera.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/Helper.hpp"

#include "Object/WorldObject.hpp"

struct ParticleStyle
{
    sf::Vector2f size;
    sf::Color startColour;
    sf::Color endColour;
    float lifetimeMin;
    float lifetimeMax;
};

class Particle : public WorldObject
{
public:
    Particle(sf::Vector2f position, sf::Vector2f velocity, sf::Vector2f acceleration, const ParticleStyle& style);

    void update(float dt);

    void draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const;

    bool isAlive();

    sf::Vector2f getPositionDrawOffset() const;

private:
    sf::Vector2f velocity;
    sf::Vector2f acceleration;

    float lifetime;
    float timeAlive;

    sf::Vector2f size;

    sf::Color startColour;
    sf::Color endColour;
};

class ParticleSystem
{
public:
    ParticleSystem() = default;

    void addParticle(const Particle& particle);

    void update(float dt);

    std::vector<WorldObject*> getParticles();

private:
    std::vector<Particle> particles;
};