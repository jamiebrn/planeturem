#include "Object/ParticleSystem.hpp"

Particle::Particle(sf::Vector2f position, sf::Vector2f velocity, sf::Vector2f acceleration, const ParticleStyle& style)
    : WorldObject(position)
{
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

void Particle::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    // For now, will use texture for particles in future
    spriteBatch.endDrawing(window);

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

sf::Vector2f Particle::getPositionDrawOffset() const
{
    return sf::Vector2f(0, 0);
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

std::vector<WorldObject*> ParticleSystem::getParticles()
{
    std::vector<WorldObject*> particleObjects;
    for (int i = 0; i < particles.size(); i++)
    {
        particleObjects.push_back(&particles[i]);
    }
    return particleObjects;
}