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

class Particle
{
public:
    Particle(sf::Vector2f position, sf::Vector2f velocity, sf::Vector2f acceleration, const ParticleStyle& style);

    void update(float dt);

    void draw(sf::RenderTarget& window) const;

    bool isAlive();

private:
    sf::Vector2f position;
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

    void draw(sf::RenderTarget& window) const;

private:
    std::vector<Particle> particles;
};