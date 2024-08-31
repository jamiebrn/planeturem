#include "Object/BuildableObject.hpp"

BuildableObject::BuildableObject(sf::Vector2f position, ObjectType objectType)
    : WorldObject(position)
{
    this->objectType = objectType;

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    health = objectData.health;
    flash_amount = 0.0f;

    drawLayer = objectData.drawLayer;
}

BuildableObject::BuildableObject(ObjectReference _objectReference)
    : WorldObject({0, 0})
{
    objectReference = _objectReference;
}

void BuildableObject::update(float dt, bool onWater)
{
    flash_amount = std::max(flash_amount - dt * 3, 0.0f);

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    animatedTexture.update(dt, objectData.textureRects.size(), objectData.textureFrameDelay);

    this->onWater = onWater;
}

void BuildableObject::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime, int worldSize, const sf::Color& color)
{
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
        spriteBatch.draw(window, drawData, static_cast<sf::FloatRect>(textureRect));
    }
    else
    {
        // End batch
        spriteBatch.endDrawing(window);

        sf::Shader* shader = Shaders::getShader(ShaderType::Flash);
        shader->setUniform("flash_amount", flash_amount);

        TextureManager::drawSubTexture(window, drawData, textureRect, shader);
    }
}

void BuildableObject::drawGUI(sf::RenderTarget& window, float dt, const sf::Color& color)
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    TextureManager::drawSubTexture(window, {
        TextureType::Objects, position, 0, {2, 2}, {0.5, 0.5}, color
        }, objectData.textureRects[0]);
}

bool BuildableObject::damage(int amount, InventoryData& inventory)
{
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

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    if (objectData.chestCapacity > 0)
    {
        // Object is chest
        interactionData.chestID = chestID;
        interactionData.interactionType = ObjectInteraction::Chest;
    }

    return interactionData;
}

void BuildableObject::setWorldPosition(sf::Vector2f position)
{
    this->position = position;
}

// Chest functionality
int BuildableObject::getChestCapactity()
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    return objectData.chestCapacity;
}