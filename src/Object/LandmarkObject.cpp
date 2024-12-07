#include "Object/LandmarkObject.hpp"
#include "Game.hpp"

LandmarkObject::LandmarkObject(sf::Vector2f position, ObjectType objectType, Game& game)
    : BuildableObject(position, objectType)
{
    game.landmarkPlaced(*this);
}

BuildableObject* LandmarkObject::clone()
{
    return new LandmarkObject(*this);
}

void LandmarkObject::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const sf::Color& color) const
{
    drawObject(window, spriteBatch, camera, gameTime, worldSize, color);

    float waterYOffset = getWaterBobYOffset(worldSize, gameTime);

    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    assert(objectData.landmarkObjectData.has_value());

    // Draw landmark colour piece
    sf::Shader* replaceShader = Shaders::getShader(ShaderType::ReplaceColour);

    replaceShader->setUniform("toReplace", sf::Glsl::Vec4(sf::Color(255, 255, 255)));
    replaceShader->setUniform("replaceColour", sf::Glsl::Vec4(objectData.landmarkObjectData->colour));

    TextureDrawData drawData;
    drawData.type = TextureType::Objects;
    drawData.position = camera.worldToScreenTransform(position + sf::Vector2f(0, waterYOffset));
    drawData.scale = sf::Vector2f(ResolutionHandler::getScale(), ResolutionHandler::getScale());
    drawData.centerRatio = objectData.textureOrigin;

    spriteBatch.draw(window, drawData, sf::IntRect(objectData.landmarkObjectData->bloomBitmaskOffset, objectData.textureRects[0].getSize()), ShaderType::ReplaceColour);
}

bool LandmarkObject::damage(int amount, Game& game, InventoryData& inventory, bool giveItems)
{
    bool destroyed = BuildableObject::damage(amount, game, inventory);

    if (destroyed)
    {
        game.landmarkDestroyed(*this);
    }

    return destroyed;
}