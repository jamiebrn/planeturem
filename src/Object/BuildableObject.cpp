#include "Object/BuildableObject.hpp"
#include "World/ChunkManager.hpp"
#include "Game.hpp"

BuildableObject::BuildableObject(sf::Vector2f position, ObjectType objectType, bool randomiseAnimation)
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
    
    flash_amount = 0.0f;

    drawLayer = objectData.drawLayer;

    // Initialise chest
    // if (objectData.chestCapacity > 0)
    // {
    //     animationDirection = -1;
    //     randomiseAnimation = false;
    // }

    // Randomise animation start frame
    if (randomiseAnimation && objectData.textureRects.size() > 0)
    {
        animatedTexture.setFrame(rand() % objectData.textureRects.size());
    }
}

BuildableObject* BuildableObject::clone()
{
    return new BuildableObject(*this);
}

BuildableObject::BuildableObject(ObjectReference _objectReference)
    : WorldObject({0, 0})
{
    objectReference = _objectReference;
}

void BuildableObject::update(Game& game, float dt, bool onWater, bool loopAnimation)
{
    if (objectType < 0 || isObjectReference())
        return;

    flash_amount = std::max(flash_amount - dt * 3, 0.0f);

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    animatedTexture.update(dt, animationDirection, objectData.textureRects.size(), objectData.textureFrameDelay, loopAnimation);

    this->onWater = onWater;
}

void BuildableObject::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const sf::Color& color) const
{
    if (objectType < 0)
        return;

    drawObject(window, spriteBatch, camera, gameTime, worldSize, color);
}

void BuildableObject::drawObject(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Camera& camera, float gameTime, int worldSize, const sf::Color& color,
    std::optional<std::vector<sf::IntRect>> textureRectsOverride, std::optional<sf::Vector2f> textureOriginOverride, const sf::Texture* textureOverride) const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    float scaleMult = 0.4f * std::sin(3.14 / 2.0f * std::max(1.0f - flash_amount, 0.5f)) + 0.6f;
    sf::Vector2f scale = sf::Vector2f((float)ResolutionHandler::getScale(), (float)ResolutionHandler::getScale() * scaleMult);

    const sf::IntRect* textureRect = nullptr;
    if (textureRectsOverride.has_value())
    {
        textureRect = &textureRectsOverride->at(animatedTexture.getFrame());
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

    TextureDrawData drawData = {
        TextureType::Objects, camera.worldToScreenTransform(position + sf::Vector2f(0, waterYOffset)), 0, scale, objectData.textureOrigin, color
        };
    
    if (textureOriginOverride.has_value())
    {
        drawData.centerRatio = textureOriginOverride.value();
    }
    
    std::optional<ShaderType> shaderType;

    if (flash_amount > 0)
    {
        shaderType = ShaderType::Flash;
        sf::Shader* shader = Shaders::getShader(shaderType.value());
        shader->setUniform("flash_amount", flash_amount);
    }

    if (textureOverride)
    {
        spriteBatch.endDrawing(window);

        sf::Sprite textureSprite;
        textureSprite.setTexture(*textureOverride);
        textureSprite.setPosition(drawData.position);
        textureSprite.setScale(drawData.scale);
        textureSprite.setOrigin(sf::Vector2f(textureOverride->getSize().x * drawData.centerRatio.x, textureOverride->getSize().y * drawData.centerRatio.y));
        textureSprite.setColor(drawData.colour);

        sf::Shader* shader = nullptr;
        if (shaderType.has_value())
        {
            shader = Shaders::getShader(shaderType.value());
        }

        window.draw(textureSprite, shader);
    }
    else
    {
        spriteBatch.draw(window, drawData, *textureRect, shaderType);
    }
}

void BuildableObject::createLightSource(LightingEngine& lightingEngine, sf::Vector2f topLeftChunkPos) const
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
    sf::Vector2f topLeftRelativePos = position - topLeftChunkPos;
    
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

    // // Calculate light position based on object size
    // sf::Vector2f lightPos = position;
    // if (objectData.size != sf::Vector2i(1, 1))
    // {
    //     sf::Vector2f topLeftPos = position - sf::Vector2f(0.5f, 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
    //     lightPos = topLeftPos + TILE_SIZE_PIXELS_UNSCALED * static_cast<sf::Vector2f>(objectData.size) / 2.0f;
    // }

    // // Draw light
    // static const sf::Color lightColor(255, 220, 140);
    // float lightScale = 0.3f * objectData.lightEmission;

    // sf::Vector2f scale((float)ResolutionHandler::getScale() * lightScale, (float)ResolutionHandler::getScale() * lightScale);

    // sf::IntRect lightMaskRect(0, 0, 256, 256);

    // TextureManager::drawSubTexture(lightTexture, {
    //     TextureType::LightMask, Camera::worldToScreenTransform(lightPos), 0, scale, {0.5, 0.5}, lightColor
    //     }, lightMaskRect, sf::BlendAdd);
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

    flash_amount = 1.0f;
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
            createItemPickups(chunkManager, objectData.itemDrops, game.getGameTime());
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
    style.textureRects = {sf::IntRect(0, 384, 16, 16), sf::IntRect(16, 384, 16, 16), sf::IntRect(32, 384, 16, 16), sf::IntRect(48, 384, 16, 16)};
    style.timePerFrame = 0.08f;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    // sf::Vector2f objectCentre = position + sf::Vector2f(objectData.size.x / 2.0f - 0.5f, objectData.size.y / 2.0f - 0.5f) * TILE_SIZE_PIXELS_UNSCALED;

    static const float PARTICLE_SPEED = 60.0f;

    // Create particles on edges
    for (int x = 0; x < objectData.size.x; x++)
    {
        static constexpr float upRotation = 0.75f * 2 * M_PI;
        static constexpr float downRotation = 0.25f * 2 * M_PI;

        particleSystem.addParticle(Particle(position + sf::Vector2f(x, 0) * TILE_SIZE_PIXELS_UNSCALED,
            sf::Vector2f(std::cos(upRotation), std::sin(upRotation)) * PARTICLE_SPEED, sf::Vector2f(0, 0), style));
        particleSystem.addParticle(Particle(position + sf::Vector2f(x, objectData.size.y - 1) * TILE_SIZE_PIXELS_UNSCALED,
            sf::Vector2f(std::cos(downRotation), std::sin(downRotation)) * PARTICLE_SPEED, sf::Vector2f(0, 0), style));
    }

    for (int y = 0; y < objectData.size.y; y++)
    {
        static constexpr float leftRotation = 0.5f * 2 * M_PI;
        static constexpr float rightRotation = 1.0f * 2 * M_PI;

        particleSystem.addParticle(Particle(position + sf::Vector2f(0, y) * TILE_SIZE_PIXELS_UNSCALED,
            sf::Vector2f(std::cos(leftRotation), std::sin(leftRotation)) * PARTICLE_SPEED, sf::Vector2f(0, 0), style));
        particleSystem.addParticle(Particle(position + sf::Vector2f(objectData.size.x - 1, y) * TILE_SIZE_PIXELS_UNSCALED,
            sf::Vector2f(std::cos(rightRotation), std::sin(rightRotation)) * PARTICLE_SPEED, sf::Vector2f(0, 0), style));
    }

    // Create particles on corners
    particleSystem.addParticle(Particle(position, sf::Vector2f(std::cos(5 / 8.0f * 2 * M_PI), std::sin(5 / 8.0f * 2 * M_PI)) * PARTICLE_SPEED, sf::Vector2f(0, 0), style));
    particleSystem.addParticle(Particle(position + sf::Vector2f(objectData.size.x - 1, 0) * TILE_SIZE_PIXELS_UNSCALED,
        sf::Vector2f(std::cos(7 / 8.0f * 2 * M_PI), std::sin(7 / 8.0f * 2 * M_PI)) * PARTICLE_SPEED, sf::Vector2f(0, 0), style));
    particleSystem.addParticle(Particle(position + sf::Vector2f(objectData.size.x - 1, objectData.size.y - 1) * TILE_SIZE_PIXELS_UNSCALED,
        sf::Vector2f(std::cos(1 / 8.0f * 2 * M_PI), std::sin(1 / 8.0f * 2 * M_PI)) * PARTICLE_SPEED, sf::Vector2f(0, 0), style));
    particleSystem.addParticle(Particle(position + sf::Vector2f(0, objectData.size.y - 1) * TILE_SIZE_PIXELS_UNSCALED,
        sf::Vector2f(std::cos(3 / 8.0f * 2 * M_PI), std::sin(3 / 8.0f * 2 * M_PI)) * PARTICLE_SPEED, sf::Vector2f(0, 0), style));
}

void BuildableObject::createHitMarker(int amount)
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    sf::Vector2f spawnPos = position - sf::Vector2f(0.5f, 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
    spawnPos.x += Helper::randFloat(0.0f, objectData.size.x * TILE_SIZE_PIXELS_UNSCALED);
    spawnPos.y += Helper::randFloat(0.0f, objectData.size.y * TILE_SIZE_PIXELS_UNSCALED);

    sf::Color hitColour = sf::Color(247, 150, 23);
    if (amount <= 0)
    {
        hitColour = sf::Color(208, 15, 30);
    }

    HitMarkers::addHitMarker(spawnPos, amount, hitColour);
}

void BuildableObject::createItemPickups(ChunkManager& chunkManager, const std::vector<ItemDrop>& itemDrops, float gameTime)
{
    float dropChance = (rand() % 1000) / 1000.0f;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    for (const ItemDrop& itemDrop : itemDrops)
    {
        if (dropChance < itemDrop.chance)
        {
            // Give items
            unsigned int itemAmount = rand() % std::max(itemDrop.maxAmount - itemDrop.minAmount + 1, 1U) + itemDrop.minAmount;

            for (int i = 0; i < itemAmount; i++)
            {
                sf::Vector2f spawnPos = position - sf::Vector2f(0.5f, 0.5f) * TILE_SIZE_PIXELS_UNSCALED;
                spawnPos.x += Helper::randFloat(0.0f, objectData.size.x * TILE_SIZE_PIXELS_UNSCALED);
                spawnPos.y += Helper::randFloat(0.0f, objectData.size.y * TILE_SIZE_PIXELS_UNSCALED);

                chunkManager.addItemPickup(ItemPickup(spawnPos, itemDrop.item, gameTime));
            }

            // inventory.addItem(itemDrop.item, itemAmount, true);
        }
    }
}

void BuildableObject::interact(Game& game)
{
    // No interaction for regular object
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

}

bool BuildableObject::isInteractable() const
{
    return false;
}

void BuildableObject::triggerBehaviour(Game& game, ObjectBehaviourTrigger trigger)
{
    // No behaviour trigger for regular object
}

void BuildableObject::setWorldPosition(sf::Vector2f position)
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