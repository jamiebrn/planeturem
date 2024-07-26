#include "Object/BuildableObject.hpp"

BuildableObject::BuildableObject(sf::Vector2f position, unsigned int objectType)
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

void BuildableObject::update(float dt)
{
    flash_amount = std::max(flash_amount - dt * 3, 0.0f);
}

void BuildableObject::draw(sf::RenderWindow& window, float dt, const sf::Color& color)
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    sf::Shader* shader = Shaders::getShader(ShaderType::Flash);
    shader->setUniform("flash_amount", flash_amount);
    TextureManager::drawSubTexture(window, {
        TextureType::BuildItems, position + Camera::getIntegerDrawOffset(), 0, 3, objectData.textureOrigin, color
        }, objectData.textureRect, shader);
}

void BuildableObject::drawGUI(sf::RenderWindow& window, float dt, const sf::Color& color)
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    TextureManager::drawSubTexture(window, {
        TextureType::BuildItems, position, 0, 3, {0.5, 0.5}, color
        }, objectData.textureRect);
}

void BuildableObject::interact()
{
    flash_amount = 1.0f;
    health -= 1;

    if (!isAlive())
    {
        // Give item drops
        const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);
        for (auto& itemDropPair : objectData.itemDrops)
        {
            Inventory::addItem(itemDropPair.first, itemDropPair.second);
        }
    }
}