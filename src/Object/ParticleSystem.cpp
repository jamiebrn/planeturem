#include "Object/ParticleSystem.hpp"

Particle::Particle(sf::Vector2f position, sf::Vector2f velocity, sf::Vector2f acceleration, const ParticleStyle& style)
{
    this->position = position;
    this->velocity = velocity;
    this->acceleration = acceleration;

    textureRects = style.textureRects;
    currentFrame = 0;
    frameTimer = 0.0f;
    timePerFrame = style.timePerFrame;
}

void Particle::update(float dt)
{
    velocity += acceleration * dt;
    position += velocity * dt;

    frameTimer += dt;
    if (frameTimer >= timePerFrame)
    {
        frameTimer = 0.0f;
        currentFrame++;
    }
}

void Particle::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) const
{
    float scale = ResolutionHandler::getScale();

    TextureDrawData drawData;
    drawData.position = Camera::worldToScreenTransform(position);
    drawData.type = TextureType::Objects;
    drawData.scale = sf::Vector2f(scale, scale);
    drawData.centerRatio = sf::Vector2f(0.5, 0.5);

    spriteBatch.draw(window, drawData, textureRects[std::min(currentFrame, static_cast<int>(textureRects.size()) - 1)]);
}

bool Particle::isAlive()
{
    return (currentFrame < textureRects.size());
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

void ParticleSystem::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch) const
{
    for (auto iter = particles.begin(); iter != particles.end(); iter++)
    {
        iter->draw(window, spriteBatch);
    }
}