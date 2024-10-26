#include "Object/RocketObject.hpp"
#include "Game.hpp"

RocketObject::RocketObject(sf::Vector2f position, ObjectType objectType)
    : BuildableObject(position, objectType)
{

}

BuildableObject* RocketObject::clone()
{
    return new RocketObject(*this);
}

void RocketObject::update(Game& game, float dt, bool onWater, bool loopAnimation)
{
    BuildableObject::update(game, dt, onWater);

    particleSystem.update(dt);
    
    
    if (flyingUp || flyingDown)
    {
        floatTween.update(dt);
        
        if (floatTween.isTweenFinished(rocketFlyingTweenID))
        {
            if (flyingUp)
            {
                flyingUp = false;
                game.rocketFinishedUp(*this);
            }
            else if (flyingDown)
            {
                flyingDown = false;
                game.rocketFinishedDown(*this);
            }
        }

        rocketParticleCooldown += dt;
        if (rocketParticleCooldown >= ROCKET_PARTICLE_MAX_COOLDOWN)
        {
            rocketParticleCooldown = 0;
            createRocketParticles();
        }
    }
}

void RocketObject::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    BuildableObject::draw(window, spriteBatch, game, dt, gameTime, worldSize, color);

    // Draw rocket particles
    particleSystem.draw(window, spriteBatch);

    drawRocket(window, spriteBatch, color);
}

void RocketObject::interact(Game& game)
{
    // Rocket interaction stuff
    if (!entered)
    {
        game.enterRocket(*this);
        entered = true;
    }
}

bool RocketObject::isInteractable() const
{
    return true;
}

void RocketObject::triggerBehaviour(Game& game, ObjectBehaviourTrigger trigger)
{
    switch (trigger)
    {
        case ObjectBehaviourTrigger::RocketFlyUp:
        {
            startFlyingUpwards();
            break;
        }
        case ObjectBehaviourTrigger::RocketFlyDown:
        {
            startFlyingDownwards();
            game.enterIncomingRocket(*this);
            break;
        }
        case ObjectBehaviourTrigger::RocketExit:
        {
            entered = false;
            break;
        }
    }
}

void RocketObject::startFlyingUpwards()
{
    // Just to be sure
    entered = true;

    flyingUp = true;
    rocketYOffset = 0.0f;

    rocketFlyingTweenID = floatTween.startTween(&rocketYOffset, rocketYOffset, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE * -4, 3.0f,
        TweenTransition::Quint, TweenEasing::EaseInOut);
}

void RocketObject::startFlyingDownwards()
{
    // Just to be sure
    entered = true;

    flyingDown = true;
    rocketYOffset = TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE * -4;

    rocketFlyingTweenID = floatTween.startTween(&rocketYOffset, rocketYOffset, 0.0f, 3.0f,
        TweenTransition::Quint, TweenEasing::EaseInOut);
}

std::vector<PlanetType> RocketObject::getRocketAvailableDestinations(PlanetType currentPlanetType)
{
    std::vector<PlanetType> availableDestinations;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    for (PlanetType destination : objectData.rocketObjectData->availableDestinations)
    {
        if (destination == currentPlanetType)
            continue;
        
        availableDestinations.push_back(destination);
    }

    return availableDestinations;
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