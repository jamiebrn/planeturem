#include "Object/ParticleSystem.hpp"

Particle::Particle(sf::Vector2f position, sf::Vector2f velocity, sf::Vector2f acceleration, const ParticleStyle& style)
{
    this->position = position;
    this->velocity = velocity;
    this->acceleration = acceleration;

    lifetime = Helper::randFloat(style.lifetimeMin, style.lifetimeMax);
    timeAlive = 0.0f;

    size = style.size;
    startColour = style.startColour;
    endColour = style.endColour;
}

void Particle::update(float dt)
{
    velocity += acceleration * dt;
    position += velocity * dt;

    timeAlive += dt;
}

void Particle::draw(sf::RenderTarget& window) const
{
    float scale = ResolutionHandler::getScale();

    sf::RectangleShape rect(size * scale);
    rect.setPosition(Camera::worldToScreenTransform(position));
    
    float progress = timeAlive / lifetime;
    sf::Color drawColour;
    drawColour.r = (endColour.r - startColour.r) * progress + startColour.r;
    drawColour.g = (endColour.g - startColour.g) * progress + startColour.g;
    drawColour.b = (endColour.b - startColour.b) * progress + startColour.b;
    drawColour.a = (endColour.a - startColour.a) * progress + startColour.a;

    rect.setFillColor(drawColour);

    window.draw(rect);
}

bool Particle::isAlive()
{
    return (timeAlive < lifetime);
}

void ParticleSystem::addParticle(const Particle& particle)
{
    particles.push_back(particle);
}

void ParticleSystem::update(float dt)
{
    for (auto iter = particles.begin(); iter != particles.end();)
    {
        iter->update(dt);
        if (!iter->isAlive())
        {
            iter = particles.erase(iter);
            continue;
        }
        iter++;
    }
}

void ParticleSystem::draw(sf::RenderTarget& window) const
{
    for (auto iter = particles.begin(); iter != particles.end(); iter++)
    {
        iter->draw(window);
    }
}