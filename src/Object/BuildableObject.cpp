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
    // if (objectData.chestCapacity > 0)
    // {
    //     animationDirection = -1;
    //     randomiseAnimation = false;
    // }

    // Randomise animation start frame
    if (randomiseAnimation)
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
    if (objectType < 0)
        return;

    flash_amount = std::max(flash_amount - dt * 3, 0.0f);

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    animatedTexture.update(dt, animationDirection, objectData.textureRects.size(), objectData.textureFrameDelay, loopAnimation);

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

    // if (flash_amount <= 0)
    // {
    // }
    // else
    // {
        // End batch
        // spriteBatch.endDrawing(window);
    
    std::optional<ShaderType> shaderType;

    if (flash_amount > 0)
    {
        shaderType = ShaderType::Flash;
        sf::Shader* shader = Shaders::getShader(shaderType.value());
        shader->setUniform("flash_amount", flash_amount);
    }

    spriteBatch.draw(window, drawData, textureRect, shaderType);
    
        // TextureManager::drawSubTexture(window, drawData, textureRect, shader);
    // }
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

void BuildableObject::interact(Game& game)
{
    // No interaction for regular object
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