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

    particleSystem.update(dt);
    
    if (flying)
    {
        floatTween.update(dt);

        if (floatTween.isTweenFinished(rocketFlyingTweenID))
        {
            flying = false;
        }
        
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
    // spriteBatch.endDrawing(window);

    // Draw rocket particles
    particleSystem.draw(window, spriteBatch);

    drawRocket(window, spriteBatch, color);
}

ObjectInteractionType RocketObject::interact() const
{
    return ObjectInteractionType::Rocket;
}

void RocketObject::startFlyingUpwards()
{
    flying = true;
    rocketFlyingTweenID = floatTween.startTween(&rocketYOffset, 0.0f, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE * -4, 3.0f,
        TweenTransition::Quint, TweenEasing::EaseInOut);
}

void RocketObject::startFlyingDownwards()
{
    flying = true;
    rocketFlyingTweenID = floatTween.startTween(&rocketYOffset, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE * -4, 0.0f, 3.0f,
        TweenTransition::Quint, TweenEasing::EaseInOut);
}

bool RocketObject::finishedFlying()
{
    if (!flying)
        return true;
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
    style.timePerFrame = Helper::randFloat(0.05f, 0.4f);
    
    int particleType = Helper::randInt(0, 2);

    for (int i = 0; i < 4; i++)
    {
        sf::IntRect textureRect;
        textureRect.left = i * 16;
        textureRect.top = 384 + particleType * 16;
        textureRect.height = 16;
        textureRect.width = 16;
        style.textureRects.push_back(textureRect);
    }

    sf::Vector2f position = getRocketBottomPosition() + sf::Vector2f(Helper::randInt(-1, 1), Helper::randInt(-1, 1));
    sf::Vector2f velocity(Helper::randFloat(-30.0f, 30.0f), Helper::randFloat(15.0f, 30.0f));
    sf::Vector2f acceleration(Helper::randFloat(-0.6, 0.6), -Helper::randFloat(0.4f, 1.0f));

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