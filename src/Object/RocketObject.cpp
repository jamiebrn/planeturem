#include "Object/RocketObject.hpp"
#include "Game.hpp"

RocketObject::RocketObject(pl::Vector2f position, ObjectType objectType)
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

void RocketObject::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const
{
    BuildableObject::draw(window, spriteBatch, game, camera, dt, gameTime, worldSize, color);

    // Draw rocket particles
    particleSystem.draw(window, spriteBatch, camera);

    drawRocket(window, spriteBatch, camera, color);
}

void RocketObject::interact(Game& game, bool isClient)
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

void RocketObject::getRocketAvailableDestinations(PlanetType currentPlanetType, RoomType currentRoomType,
    std::vector<PlanetType>& planetDestinations, std::vector<RoomType>& roomDestinations)
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    for (PlanetType planetDestination : objectData.rocketObjectData->availableDestinations)
    {
        if (planetDestination == currentPlanetType)
            continue;
        
        planetDestinations.push_back(planetDestination);
    }

    for (RoomType roomDestination : objectData.rocketObjectData->availableRoomDestinations)
    {
        if (roomDestination == currentRoomType)
            continue;
        
        roomDestinations.push_back(roomDestination);
    }
}

pl::Vector2f RocketObject::getRocketPosition()
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    
    if (!objectData.rocketObjectData.has_value())
        return position;
    
    pl::Vector2f rocketPos;
    rocketPos.x = position.x + objectData.rocketObjectData->launchPosition.x - TILE_SIZE_PIXELS_UNSCALED * 0.5f;
    rocketPos.y = position.y + objectData.rocketObjectData->launchPosition.y - TILE_SIZE_PIXELS_UNSCALED * 0.5f;

    // Rocket height
    rocketPos.y -= objectData.rocketObjectData->textureRect.height * 0.5f;

    return rocketPos;
}

pl::Vector2f RocketObject::getRocketBottomPosition()
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
    
    if (!objectData.rocketObjectData.has_value())
        return position;
    
    pl::Vector2f rocketPos;
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
        pl::Rect<int> textureRect;
        textureRect.x = i * 16;
        textureRect.y = 384 + particleType * 16;
        textureRect.height = 16;
        textureRect.width = 16;
        style.textureRects.push_back(textureRect);
    }

    pl::Vector2f position = getRocketBottomPosition() + pl::Vector2f(Helper::randInt(-1, 1), Helper::randInt(-1, 1));
    pl::Vector2f velocity(Helper::randFloat(-30.0f, 30.0f), Helper::randFloat(15.0f, 30.0f));
    pl::Vector2f acceleration(Helper::randFloat(-0.6, 0.6), -Helper::randFloat(0.4f, 1.0f));

    particleSystem.addParticle(Particle(position, velocity, acceleration, style));
}

void RocketObject::drawRocket(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, const pl::Color& color) const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    pl::Vector2f scale(ResolutionHandler::getScale(), ResolutionHandler::getScale());

    pl::Vector2f rocketPosOffset = objectData.rocketObjectData->launchPosition - pl::Vector2f(TILE_SIZE_PIXELS_UNSCALED, TILE_SIZE_PIXELS_UNSCALED) * 0.5f;

    pl::DrawData rocketDrawData;
    rocketDrawData.texture = TextureManager::getTexture(TextureType::Objects);
    rocketDrawData.shader = Shaders::getShader(ShaderType::Default);
    rocketDrawData.position = camera.worldToScreenTransform(position + rocketPosOffset + pl::Vector2f(0, rocketYOffset));
    rocketDrawData.color = color;
    rocketDrawData.scale = scale;
    rocketDrawData.centerRatio = objectData.rocketObjectData->textureOrigin;
    rocketDrawData.textureRect = objectData.rocketObjectData->textureRect;

    if (flash_amount > 0)
    {
        rocketDrawData.shader = Shaders::getShader(ShaderType::Flash);
        rocketDrawData.shader->setUniform1f("flash_amount", flash_amount);
    }

    spriteBatch.draw(window, rocketDrawData);
}