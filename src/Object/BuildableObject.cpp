#include "Object/BuildableObject.hpp"
#include "World/ChunkManager.hpp"
#include "Game.hpp"

BuildableObject::BuildableObject(pl::Vector2f position, ObjectType objectType, const BuildableObjectCreateParameters& parameters)
    : WorldObject(position)
{
    this->objectType = objectType;

    if (isDummyObject())
        return;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    bool useObjectHealth = !objectData.plantStageObjectData.has_value();
    if (useObjectHealth)
    {
        health = objectData.health;
    }
    
    flashAmount = parameters.flashOnCreate ? 1.0f : 0.0f;

    drawLayer = objectData.drawLayer;

    // Initialise chest
    // if (objectData.chestCapacity > 0)
    // {
    //     animationDirection = -1;
    //     randomiseAnimation = false;
    // }

    // Randomise animation start frame
    if (parameters.randomiseAnimation && objectData.textureRects.size() > 0)
    {
        animatedTexture.setFrame(rand() % objectData.textureRects.size());
    }
}

BuildableObject* BuildableObject::clone()
{
    return new BuildableObject(*this);
}

BuildableObject::BuildableObject(ObjectReference objectReference)
    : WorldObject({0, 0})
{
    this->objectReference = objectReference;
}

void BuildableObject::update(Game& game, float dt, bool onWater, bool loopAnimation)
{
    if (objectType < 0 || isObjectReference())
        return;

    flashAmount = std::max(flashAmount - dt * 3, 0.0f);

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    animatedTexture.update(dt, animationDirection, objectData.textureRects.size(), objectData.textureFrameDelay, loopAnimation);

    this->onWater = onWater;
}

void BuildableObject::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const pl::Color& color) const
{
    if (objectType < 0)
        return;

    drawObject(window, spriteBatch, camera, gameTime, worldSize, color);
}

void BuildableObject::drawObject(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Camera& camera, float gameTime, int worldSize, const pl::Color& color,
    std::optional<std::vector<pl::Rect<int>>> textureRectsOverride, std::optional<pl::Vector2f> textureOriginOverride, const pl::Texture* textureOverride) const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    float scaleMult = 0.4f * std::sin(3.14 / 2.0f * std::max(1.0f - flashAmount, 0.5f)) + 0.6f;
    pl::Vector2f scale = pl::Vector2f((float)ResolutionHandler::getScale(), (float)ResolutionHandler::getScale() * scaleMult);

    const pl::Rect<int>* textureRect = nullptr;
    if (textureRectsOverride.has_value())
    {
        textureRect = &textureRectsOverride->at(std::min(animatedTexture.getFrame(), static_cast<int>(textureRectsOverride->size()) - 1));
    }
    else
    {
        textureRect = &objectData.textureRects[animatedTexture.getFrame()];
    }

    if (!textureRect)
    {
        return;
    }

    float waterYOffset = getWaterBobYOffset(worldSize, gameTime);

    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::Objects);
    drawData.shader = Shaders::getShader(ShaderType::Default);
    drawData.position = camera.worldToScreenTransform(position + pl::Vector2f(0, waterYOffset), worldSize);
    drawData.scale = scale;
    drawData.centerRatio = objectData.textureOrigin;
    drawData.color = color;
    drawData.textureRect = *textureRect;
    
    if (textureOriginOverride.has_value())
    {
        drawData.centerRatio = textureOriginOverride.value();
    }

    if (textureOverride)
    {
        drawData.texture = textureOverride;
    }

    if (flashAmount > 0)
    {
        drawData.shader = Shaders::getShader(ShaderType::Flash);
        drawData.shader->setUniform1f("flash_amount", flashAmount);
    }

    spriteBatch.draw(window, drawData);
}

void BuildableObject::createLightSource(LightingEngine& lightingEngine, pl::Vector2f topLeftChunkPos, pl::Vector2f playerPos, int worldSize) const
{
    if (objectType < 0)
    {
        return;
    }

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    void (LightingEngine::*lightingFunction)(int, int, float) = nullptr;
    float lightingValue = 0.0f;

    if (objectData.lightEmissionFrames.size() > 0)
    {
        lightingFunction = &LightingEngine::addLightSource;
        lightingValue = objectData.lightEmissionFrames[animatedTexture.getFrame()];
    }
    else if (objectData.lightAbsorption > 0)
    {
        lightingFunction = &LightingEngine::addObstacle;
        lightingValue = objectData.lightAbsorption;
    }

    if (!lightingFunction)
    {
        return;
    }

    // Create light emitter / absorber
    pl::Vector2f topLeftRelativePos = Camera::translateWorldPos(position, playerPos, worldSize) - topLeftChunkPos;
    
    int lightingTileX = std::floor(topLeftRelativePos.x / TILE_SIZE_PIXELS_UNSCALED) * TILE_LIGHTING_RESOLUTION;
    int lightingTileY = std::floor(topLeftRelativePos.y / TILE_SIZE_PIXELS_UNSCALED) * TILE_LIGHTING_RESOLUTION;

    // Create light sources for all required tiles
    for (int x = 0; x < objectData.size.x * TILE_LIGHTING_RESOLUTION; x++)
    {
        for (int y = 0; y < objectData.size.y * TILE_LIGHTING_RESOLUTION; y++)
        {
            (lightingEngine.*lightingFunction)(lightingTileX + x, lightingTileY + y, lightingValue);
        }
    }
}

bool BuildableObject::damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem& particleSystem, bool giveItems)
{
    if (objectType < 0)
        return false;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    if (amount < objectData.minimumDamage)
    {
        createHitMarker(0);
        return false;
    }

    flashAmount = 1.0f;
    health -= amount;

    // Play hit sound
    SoundType hitSound = SoundType::HitObject;
    int soundChance = rand() % 3;
    if (soundChance == 1) hitSound = SoundType::HitObject2;
    else if (soundChance == 2) hitSound = SoundType::HitObject3;

    Sounds::playSound(hitSound, 60.0f);

    createHitParticles(particleSystem);
    createHitMarker(amount);

    if (!isAlive())
    {
        if (giveItems)
        {
            // Give item drops
            createItemPickups(chunkManager, game, objectData.itemDrops, game.getGameTime());
        }

        return true;
    }

    return false;
}

void BuildableObject::createHitParticles(ParticleSystem& particleSystem)
{
    if (objectType < 0)
    {
        return;
    }

    ParticleStyle style;
    style.textureRects = {pl::Rect<int>(0, 384, 16, 16), pl::Rect<int>(16, 384, 16, 16), pl::Rect<int>(32, 384, 16, 16), pl::Rect<int>(48, 384, 16, 16)};
    style.timePerFrame = 0.08f;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    // pl::Vector2f objectCentre = position + pl::Vector2f(objectData.size.x / 2.0f - 0.5f, objectData.size.y / 2.0f - 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    static const float PARTICLE_SPEED = 60.0f;

    // Create particles on edges
    for (int x = 0; x < objectData.size.x; x++)
    {
        static constexpr float upRotation = 0.75f * 2 * M_PI;
        static constexpr float downRotation = 0.25f * 2 * M_PI;

        particleSystem.addParticle(Particle(position + pl::Vector2f(x, 0) * TILE_SIZE_PIXELS_UNSCALED,
            pl::Vector2f(std::cos(upRotation), std::sin(upRotation)) * PARTICLE_SPEED, pl::Vector2f(0, 0), style));
        particleSystem.addParticle(Particle(position + pl::Vector2f(x, objectData.size.y - 1) * TILE_SIZE_PIXELS_UNSCALED,
            pl::Vector2f(std::cos(downRotation), std::sin(downRotation)) * PARTICLE_SPEED, pl::Vector2f(0, 0), style));
    }

    for (int y = 0; y < objectData.size.y; y++)
    {
        static constexpr float leftRotation = 0.5f * 2 * M_PI;
        static constexpr float rightRotation = 1.0f * 2 * M_PI;

        particleSystem.addParticle(Particle(position + pl::Vector2f(0, y) * TILE_SIZE_PIXELS_UNSCALED,
            pl::Vector2f(std::cos(leftRotation), std::sin(leftRotation)) * PARTICLE_SPEED, pl::Vector2f(0, 0), style));
        particleSystem.addParticle(Particle(position + pl::Vector2f(objectData.size.x - 1, y) * TILE_SIZE_PIXELS_UNSCALED,
            pl::Vector2f(std::cos(rightRotation), std::sin(rightRotation)) * PARTICLE_SPEED, pl::Vector2f(0, 0), style));
    }

    // Create particles on corners
    particleSystem.addParticle(Particle(position, pl::Vector2f(std::cos(5 / 8.0f * 2 * M_PI), std::sin(5 / 8.0f * 2 * M_PI)) * PARTICLE_SPEED, pl::Vector2f(0, 0), style));
    particleSystem.addParticle(Particle(position + pl::Vector2f(objectData.size.x - 1, 0) * TILE_SIZE_PIXELS_UNSCALED,
        pl::Vector2f(std::cos(7 / 8.0f * 2 * M_PI), std::sin(7 / 8.0f * 2 * M_PI)) * PARTICLE_SPEED, pl::Vector2f(0, 0), style));
    particleSystem.addParticle(Particle(position + pl::Vector2f(objectData.size.x - 1, objectData.size.y - 1) * TILE_SIZE_PIXELS_UNSCALED,
        pl::Vector2f(std::cos(1 / 8.0f * 2 * M_PI), std::sin(1 / 8.0f * 2 * M_PI)) * PARTICLE_SPEED, pl::Vector2f(0, 0), style));
    particleSystem.addParticle(Particle(position + pl::Vector2f(0, objectData.size.y - 1) * TILE_SIZE_PIXELS_UNSCALED,
        pl::Vector2f(std::cos(3 / 8.0f * 2 * M_PI), std::sin(3 / 8.0f * 2 * M_PI)) * PARTICLE_SPEED, pl::Vector2f(0, 0), style));
}

void BuildableObject::createHitMarker(int amount)
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    pl::Vector2f spawnPos = position - pl::Vector2f(0.5f, 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
    spawnPos.x += Helper::randFloat(0.0f, objectData.size.x * TILE_SIZE_PIXELS_UNSCALED);
    spawnPos.y += Helper::randFloat(0.0f, objectData.size.y * TILE_SIZE_PIXELS_UNSCALED);

    pl::Color hitColour = pl::Color(247, 150, 23);
    if (amount <= 0)
    {
        hitColour = pl::Color(208, 15, 30);
    }

    HitMarkers::addHitMarker(spawnPos, amount, hitColour);
}

void BuildableObject::createItemPickups(ChunkManager& chunkManager, Game& game, const std::vector<ItemDrop>& itemDrops, float gameTime)
{
    float dropChance = (rand() % 1000) / 1000.0f;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    for (const ItemDrop& itemDrop : itemDrops)
    {
        if (dropChance < itemDrop.chance)
        {
            // Give items
            unsigned int itemAmount = rand() % std::max(itemDrop.maxAmount - itemDrop.minAmount + 1, 1U) + itemDrop.minAmount;

            pl::Vector2f spawnPos = position - pl::Vector2f(0.5f, 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
            spawnPos.x += Helper::randFloat(0.0f, objectData.size.x * TILE_SIZE_PIXELS_UNSCALED);
            spawnPos.y += Helper::randFloat(0.0f, objectData.size.y * TILE_SIZE_PIXELS_UNSCALED);

            chunkManager.addItemPickup(ItemPickup(spawnPos, itemDrop.item, gameTime, itemAmount), &game.getNetworkHandler());
        }
    }
}

void BuildableObject::interact(Game& game, bool isClient)
{
    // No interaction for regular object
    //const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

}

bool BuildableObject::isInteractable() const
{
    return false;
}

void BuildableObject::setWorldPosition(pl::Vector2f position)
{
    this->position = position;
}

// Dummy object
bool BuildableObject::isDummyObject()
{
    return (objectType == DUMMY_OBJECT_COLLISION || objectType == DUMMY_OBJECT_NO_COLLISION);
}

bool BuildableObject::dummyHasCollision()
{
    return (objectType == DUMMY_OBJECT_COLLISION);
}


// Save / load
BuildableObjectPOD BuildableObject::getPOD() const
{
    BuildableObjectPOD pod;
    pod.objectType = objectType;
    pod.objectReference = objectReference;
    return pod;
}

void BuildableObject::loadFromPOD(const BuildableObjectPOD& pod)
{
    objectType = pod.objectType;
    objectReference = pod.objectReference;
}