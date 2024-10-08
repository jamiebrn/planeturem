#include "Object/RocketObject.hpp"

RocketObject::RocketObject(sf::Vector2f position, ObjectType objectType)
    : BuildableObject(position, objectType)
{

}

BuildableObject* RocketObject::clone()
{
    return new RocketObject(*this);
}

void RocketObject::update(float dt, bool onWater, bool loopAnimation)
{
    BuildableObject::update(dt, onWater);

    if (flying)
    {
        floatTween.update(dt);

        particleSystem.update(dt);

        rocketParticleCooldown += dt;
        if (rocketParticleCooldown >= ROCKET_PARTICLE_MAX_COOLDOWN)
        {
            rocketParticleCooldown = 0;
            createRocketParticles();
        }
    }
}

void RocketObject::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    BuildableObject::draw(window, spriteBatch, dt, gameTime, worldSize, color);

    // Force spritebatch end to ensure particles draw over launch pad
    spriteBatch.endDrawing(window);

    // Draw rocket particles
    particleSystem.draw(window);

    drawRocket(window, spriteBatch, color);
}

ObjectInteractionType RocketObject::interact() const
{
    return ObjectInteractionType::Rocket;
}

void RocketObject::startFlying()
{
    flying = true;
    rocketFlyingTweenID = floatTween.startTween(&rocketYOffset, 0.0f, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE * -5, 3.0f,
        TweenTransition::Quint, TweenEasing::EaseInOut);
}

bool RocketObject::finishedFlyingUpwards()
{
    if (!flying)
        return false;
    return floatTween.isTweenFinished(rocketFlyingTweenID);
}

sf::Vector2f RocketObject::getRocketPosition()
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    
    if (!objectData.rocketObjectData.has_value())
        return position;
    
    sf::Vector2f rocketPos;
    rocketPos.x = position.x + objectData.rocketObjectData->launchPosition.x - TILE_SIZE_PIXELS_UNSCALED * 0.5f;
    rocketPos.y = position.y + objectData.rocketObjectData->launchPosition.y - TILE_SIZE_PIXELS_UNSCALED * 0.5f;

    // Rocket height
    rocketPos.y -= objectData.rocketObjectData->textureRect.height * 0.5f;

    return rocketPos;
}

sf::Vector2f RocketObject::getRocketBottomPosition()
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    
    if (!objectData.rocketObjectData.has_value())
        return position;
    
    sf::Vector2f rocketPos;
    rocketPos.x = position.x + objectData.rocketObjectData->launchPosition.x - TILE_SIZE_PIXELS_UNSCALED * 0.5f;
    rocketPos.y = position.y + objectData.rocketObjectData->launchPosition.y + rocketYOffset - TILE_SIZE_PIXELS_UNSCALED * 0.5f;

    return rocketPos;
}

void RocketObject::setRocketYOffset(float offset)
{
    rocketYOffset = offset;
}

float RocketObject::getRocketYOffset()
{
    return rocketYOffset;
}

void RocketObject::createRocketParticles()
{
    ParticleStyle style;
    float size = Helper::randFloat(3, 6);
    style.size = sf::Vector2f(size, size);
    style.lifetimeMax = 2.5f;
    style.lifetimeMin = 1.5f;
    style.startColour = sf::Color(230, 230, 230);
    style.endColour = sf::Color(40, 40, 40, 50);

    sf::Vector2f position = getRocketBottomPosition() - style.size / 2.0f;
    sf::Vector2f velocity(Helper::randFloat(-7.0f, 7.0f), Helper::randFloat(8.0f, 16.0f));
    sf::Vector2f acceleration(0, -Helper::randFloat(0.3f, 0.8f));

    particleSystem.addParticle(Particle(position, velocity, acceleration, style));
}

void RocketObject::drawRocket(sf::RenderTarget& window, SpriteBatch& spriteBatch, const sf::Color& color) const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    sf::Vector2f scale(ResolutionHandler::getScale(), ResolutionHandler::getScale());

    TextureDrawData drawData;
    drawData.type = TextureType::Objects;

    sf::Vector2f rocketPosOffset = objectData.rocketObjectData->launchPosition - sf::Vector2f(TILE_SIZE_PIXELS_UNSCALED, TILE_SIZE_PIXELS_UNSCALED) * 0.5f;
    drawData.position = Camera::worldToScreenTransform(position + rocketPosOffset + sf::Vector2f(0, rocketYOffset));
    drawData.scale = scale;
    drawData.centerRatio = objectData.rocketObjectData->textureOrigin;
    drawData.colour = color;

    spriteBatch.draw(window, drawData, objectData.rocketObjectData->textureRect);
}