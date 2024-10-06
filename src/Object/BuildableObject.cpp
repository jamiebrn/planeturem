#include "Object/BuildableObject.hpp"

BuildableObject::BuildableObject(sf::Vector2f position, ObjectType objectType, bool randomiseAnimation)
    : WorldObject(position)
{
    this->objectType = objectType;

    if (isDummyObject())
        return;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    health = objectData.health;
    flash_amount = 0.0f;

    drawLayer = objectData.drawLayer;

    // Initialise chest
    if (objectData.chestCapacity > 0)
    {
        animationDirection = -1;
        randomiseAnimation = false;
    }

    // Randomise animation start frame
    if (randomiseAnimation)
    {
        animatedTexture.setFrame(rand() % objectData.textureRects.size());
    }
}

BuildableObject::BuildableObject(ObjectReference _objectReference)
    : WorldObject({0, 0})
{
    objectReference = _objectReference;
}

void BuildableObject::update(float dt, bool onWater)
{
    if (objectType < 0)
        return;

    flash_amount = std::max(flash_amount - dt * 3, 0.0f);

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    bool loopingAnimation = true;
    if (objectData.chestCapacity > 0)
    {
        loopingAnimation = false;
    }

    animatedTexture.update(dt, animationDirection, objectData.textureRects.size(), objectData.textureFrameDelay, loopingAnimation);

    this->onWater = onWater;
}

void BuildableObject::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    if (objectType < 0)
        return;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    float scaleMult = 0.4f * std::sin(3.14 / 2.0f * std::max(1.0f - flash_amount, 0.5f)) + 0.6f;
    sf::Vector2f scale = sf::Vector2f((float)ResolutionHandler::getScale(), (float)ResolutionHandler::getScale() * scaleMult);

    const sf::IntRect& textureRect = objectData.textureRects[animatedTexture.getFrame()];

    float waterYOffset = getWaterBobYOffset(worldSize, gameTime);

    TextureDrawData drawData = {
        TextureType::Objects, Camera::worldToScreenTransform(position + sf::Vector2f(0, waterYOffset)), 0, scale, objectData.textureOrigin, color
        };

    if (flash_amount <= 0)
    {
        spriteBatch.draw(window, drawData, textureRect);
    }
    else
    {
        // End batch
        spriteBatch.endDrawing(window);

        sf::Shader* shader = Shaders::getShader(ShaderType::Flash);
        shader->setUniform("flash_amount", flash_amount);

        TextureManager::drawSubTexture(window, drawData, textureRect, shader);
    }

    // Draw rocket if required
    if (objectData.rocketObjectData.has_value())
    {
        drawRocket(window, spriteBatch, color);
    }
}

void BuildableObject::drawGUI(sf::RenderTarget& window, float dt, const sf::Color& color)
{
    if (objectType < 0)
        return;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    TextureManager::drawSubTexture(window, {
        TextureType::Objects, position, 0, {2, 2}, {0.5, 0.5}, color
        }, objectData.textureRects[0]);
}

void BuildableObject::drawRocket(sf::RenderTarget& window, SpriteBatch& spriteBatch, const sf::Color& color) const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    sf::Vector2f scale(ResolutionHandler::getScale(), ResolutionHandler::getScale());

    TextureDrawData drawData;
    drawData.type = TextureType::Objects;

    sf::Vector2f rocketPosOffset = objectData.rocketObjectData->launchPosition - sf::Vector2f(TILE_SIZE_PIXELS_UNSCALED, TILE_SIZE_PIXELS_UNSCALED) * 0.5f;
    drawData.position = Camera::worldToScreenTransform(position + rocketPosOffset);
    drawData.scale = scale;
    drawData.centerRatio = objectData.rocketObjectData->textureOrigin;
    drawData.colour = color;

    spriteBatch.draw(window, drawData, objectData.rocketObjectData->textureRect);
}

bool BuildableObject::damage(int amount, InventoryData& inventory)
{
    if (objectType < 0)
        return false;

    flash_amount = 1.0f;
    health -= amount;

    // Play hit sound
    SoundType hitSound = SoundType::HitObject;
    int soundChance = rand() % 3;
    if (soundChance == 1) hitSound = SoundType::HitObject2;
    else if (soundChance == 2) hitSound = SoundType::HitObject3;

    Sounds::playSound(hitSound, 60.0f);

    if (!isAlive())
    {
        // Give item drops
        const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
        float dropChance = (rand() % 1000) / 1000.0f;
        for (const ItemDrop& itemDrop : objectData.itemDrops)
        {
            if (dropChance < itemDrop.chance)
            {
                // Give items
                unsigned int itemAmount = rand() % std::max(itemDrop.maxAmount - itemDrop.minAmount + 1, 1U) + itemDrop.minAmount;
                inventory.addItem(itemDrop.item, itemAmount);

                // Create item popup
                InventoryGUI::pushItemPopup(ItemCount(itemDrop.item, itemAmount));
            }
        }

        return true;
    }

    return false;
}

ObjectInteractionEventData BuildableObject::interact()
{
    ObjectInteractionEventData interactionData;
    interactionData.interactionType = ObjectInteraction::NoAction;
 
    if (objectType < 0)
        return interactionData;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    if (objectData.chestCapacity > 0)
    {
        // Object is chest
        interactionData.chestID = chestID;
        interactionData.interactionType = ObjectInteraction::Chest;
    }

    if (objectData.rocketObjectData.has_value())
    {
        // Object is rocket
        interactionData.interactionType = ObjectInteraction::Rocket;
    }

    return interactionData;
}

void BuildableObject::setWorldPosition(sf::Vector2f position)
{
    this->position = position;
}

bool BuildableObject::isInteractable() const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    return (objectData.chestCapacity > 0 || objectData.rocketObjectData.has_value());
}

// Chest functionality
int BuildableObject::getChestCapactity()
{
    if (objectType < 0)
        return 0;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    return objectData.chestCapacity;
}

void BuildableObject::openChest()
{
    // Play open chest animation
    animationDirection = 1;
}

void BuildableObject::closeChest()
{
    // Rewind open chest animation
    animationDirection = -1;
}

// Rocket
sf::Vector2f BuildableObject::getRocketPosition()
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

void BuildableObject::setRocketYOffset(float offset)
{
    rocketYOffset = offset;
}

float BuildableObject::getRocketYOffset()
{
    return rocketYOffset;
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
    pod.chestID = chestID;
    pod.objectReference = objectReference;
    return pod;
}

void BuildableObject::loadFromPOD(const BuildableObjectPOD& pod)
{
    objectType = pod.objectType;
    chestID = pod.chestID;
    objectReference = pod.objectReference;
}