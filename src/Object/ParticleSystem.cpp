#include "Object/ParticleSystem.hpp"

Particle::Particle(pl::Vector2f position, pl::Vector2f velocity, pl::Vector2f acceleration, const ParticleStyle& style)
{
    this->position = position;
    this->velocity = velocity;
    this->acceleration = acceleration;

    textureRects = style.textureRects;
    alpha = style.alpha;
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

void Particle::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, int worldSize) const
{
    float scale = ResolutionHandler::getScale();

    pl::DrawData drawData;
    drawData.position = camera.worldToScreenTransform(position, worldSize);
    drawData.texture = TextureManager::getTexture(TextureType::Objects);
    drawData.shader = Shaders::getShader(ShaderType::Default);
    drawData.textureRect = textureRects[std::min(currentFrame, static_cast<int>(textureRects.size()) - 1)];
    drawData.scale = pl::Vector2f(scale, scale);
    drawData.centerRatio = pl::Vector2f(0.5, 0.5);

    // float timeAlive = currentFrame * timePerFrame + frameTimer;
    // float lifetime = textureRects.size() * timePerFrame;

    // float alpha = std::min((lifetime - timeAlive) / (lifetime * 0.2f), 1.0f);

    drawData.color = pl::Color(255, 255, 255, 255 * alpha);

    spriteBatch.draw(window, drawData);
}

bool Particle::isAlive()
{
    return (currentFrame < textureRects.size());
}

// void Particle::handleWorldWrap(pl::Vector2f positionDelta)
// {
//     position += positionDelta;
// }

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

void ParticleSystem::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, int worldSize) const
{
    for (auto iter = particles.begin(); iter != particles.end(); iter++)
    {
        iter->draw(window, spriteBatch, camera, worldSize);
    }
}

// void ParticleSystem::handleWorldWrap(pl::Vector2f positionDelta)
// {
//     for (auto& particle : particles)
//     {
//         particle.handleWorldWrap(positionDelta);
//     }
// }

void ParticleSystem::clear()
{
    particles.clear();
}