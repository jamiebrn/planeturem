#include "Object/RocketObject.hpp"
#include "Game.hpp"
#include "Network/NetworkHandler.hpp"

RocketObject::RocketObject(pl::Vector2f position, ObjectType objectType, const BuildableObjectCreateParameters& parameters)
    : BuildableObject(position, objectType, parameters)
{

}

BuildableObject* RocketObject::clone()
{
    return new RocketObject(*this);
}

void RocketObject::update(Game& game, const LocationState& locationState, float dt, bool onWater, bool loopAnimation)
{
    BuildableObject::update(game, locationState, dt, onWater);

    particleSystem.update(dt);
    
    if (flyingUp || flyingDown)
    {
        floatTween.update(dt);
        
        if (floatTween.isTweenFinished(rocketFlyingTweenID))
        {
            if (flyingUp)
            {
                flyingUp = false;
                game.rocketFinishedUp(locationState, *this);
            }
            else if (flyingDown)
            {
                flyingDown = false;
                game.rocketFinishedDown(locationState, *this);
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

bool RocketObject::damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem* particleSystem, bool giveItems, bool createHitMarkers)
{
    bool destroyed = BuildableObject::damage(amount, game, chunkManager, particleSystem, giveItems, createHitMarkers);

    if (destroyed)
    {
        game.exitRocket(LocationState::createFromPlanetType(chunkManager.getPlanetType()), this);
    }

    return destroyed;
}

void RocketObject::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const pl::Color& color) const
{
    BuildableObject::draw(window, spriteBatch, game, camera, dt, gameTime, worldSize, color);

    // Draw rocket particles
    particleSystem.draw(window, spriteBatch, camera, worldSize);

    drawRocket(window, spriteBatch, camera, worldSize, color);
}

void RocketObject::interact(Game& game, bool isClient)
{
    // Rocket interaction stuff
    if (!entered)
    {
        game.enterRocket(*this, false);
    }
}

bool RocketObject::isInteractable() const
{
    return !entered;
}

void RocketObject::startFlyingUpwards(Game& game, const LocationState& locationState, NetworkHandler* networkHandler)
{
    // Just to be sure
    entered = true;

    flyingUp = true;
    rocketYOffset = 0.0f;

    rocketFlyingTweenID = floatTween.startTween(&rocketYOffset, rocketYOffset, TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE * -4, 3.0f,
        TweenTransition::Quint, TweenEasing::EaseInOut);
    
    if (networkHandler && networkHandler->isMultiplayerGame())
    {
        PacketDataRocketInteraction packetData;
        packetData.locationState = locationState;
        packetData.rocketObjectReference = getThisObjectReference(locationState);
        packetData.interactionType = PacketDataRocketInteraction::InteractionType::FlyUp;
        
        Packet packet(packetData);
        networkHandler->sendPacketToServer(packet, k_nSteamNetworkingSend_Reliable, 0);
    }
}

void RocketObject::startFlyingDownwards(Game& game, const LocationState& locationState, NetworkHandler* networkHandler, bool enterRocket)
{
    // A player is in rocket - do not allow other players to enter
    entered = true;

    flyingDown = true;
    rocketYOffset = TILE_SIZE_PIXELS_UNSCALED * CHUNK_TILE_SIZE * -4;

    rocketFlyingTweenID = floatTween.startTween(&rocketYOffset, rocketYOffset, 0.0f, 3.0f,
        TweenTransition::Quint, TweenEasing::EaseInOut);
    
    if (enterRocket)
    {
        game.enterIncomingRocket(*this);
    }

    if (networkHandler && networkHandler->isMultiplayerGame())
    {
        PacketDataRocketInteraction packetData;
        packetData.locationState = locationState;
        packetData.rocketObjectReference = getThisObjectReference(locationState);
        packetData.interactionType = PacketDataRocketInteraction::InteractionType::FlyDown;
        
        Packet packet(packetData);
        networkHandler->sendPacketToServer(packet, k_nSteamNetworkingSend_Reliable, 0);
    }
}

void RocketObject::enter()
{
    entered = true;
}

void RocketObject::exit()
{
    entered = false;
}

bool RocketObject::isEntered()
{
    return entered;
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

void RocketObject::drawRocket(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, int worldSize, const pl::Color& color) const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    pl::Vector2f scale(ResolutionHandler::getScale(), ResolutionHandler::getScale());

    pl::Vector2f rocketPosOffset = objectData.rocketObjectData->launchPosition - pl::Vector2f(TILE_SIZE_PIXELS_UNSCALED, TILE_SIZE_PIXELS_UNSCALED) * 0.5f;

    static constexpr float FLYING_SHAKE = 1.0f;

    float alpha = 1.0f;

    pl::Vector2f worldPos = position + rocketPosOffset + pl::Vector2f(0, rocketYOffset);
    if (flyingUp || flyingDown)
    {
        worldPos.x += Helper::randFloat(-FLYING_SHAKE, FLYING_SHAKE);
    }

    pl::DrawData rocketDrawData;
    rocketDrawData.texture = TextureManager::getTexture(TextureType::Objects);
    rocketDrawData.shader = Shaders::getShader(ShaderType::Default);
    rocketDrawData.position = camera.worldToScreenTransform(worldPos, worldSize);
    rocketDrawData.color = color;
    rocketDrawData.scale = scale;
    rocketDrawData.centerRatio = objectData.rocketObjectData->textureOrigin;
    rocketDrawData.textureRect = objectData.rocketObjectData->textureRect;

    if (flashAmount > 0)
    {
        rocketDrawData.shader = Shaders::getShader(ShaderType::Flash);
        rocketDrawData.shader->setUniform1f("flash_amount", flashAmount);
    }

    spriteBatch.draw(window, rocketDrawData);
}