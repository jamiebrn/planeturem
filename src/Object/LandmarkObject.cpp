#include "Object/LandmarkObject.hpp"
#include "Game.hpp"

LandmarkObject::LandmarkObject(sf::Vector2f position, ObjectType objectType, Game& game, bool placedByPlayer)
    : BuildableObject(position, objectType)
{
    game.landmarkPlaced(*this, placedByPlayer);
}

BuildableObject* LandmarkObject::clone()
{
    return new LandmarkObject(*this);
}

void LandmarkObject::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const sf::Color& color) const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    sf::RenderTexture colouredTexture;
    colouredTexture.create(objectData.textureRects[animatedTexture.getFrame()].width, objectData.textureRects[animatedTexture.getFrame()].height);
    colouredTexture.clear(sf::Color(0, 0, 0, 0));

    sf::Glsl::Vec4 replaceKeys[2] = {sf::Glsl::Vec4(sf::Color(255, 255, 255)), sf::Glsl::Vec4(sf::Color(0, 0, 0))};
    sf::Glsl::Vec4 replaceValues[2] = {sf::Glsl::Vec4(colourA), sf::Glsl::Vec4(colourB)};

    sf::Shader* replaceColourShader = Shaders::getShader(ShaderType::ReplaceColour);
    replaceColourShader->setUniform("replaceKeyCount", 2);
    replaceColourShader->setUniformArray("replaceKeys", replaceKeys, 2);
    replaceColourShader->setUniformArray("replaceValues", replaceValues, 2);

    sf::Sprite colouredTextureSprite;
    colouredTextureSprite.setTexture(*TextureManager::getTexture(TextureType::Objects));
    colouredTextureSprite.setTextureRect(objectData.textureRects[animatedTexture.getFrame()]);

    colouredTexture.draw(colouredTextureSprite, replaceColourShader);
    colouredTexture.display();

    drawObject(window, spriteBatch, camera, gameTime, worldSize, color, std::nullopt, std::nullopt, &colouredTexture.getTexture());
}

bool LandmarkObject::damage(int amount, Game& game, InventoryData& inventory, ParticleSystem& particleSystem, bool giveItems)
{
    bool destroyed = BuildableObject::damage(amount, game, inventory, particleSystem);

    if (destroyed)
    {
        game.landmarkDestroyed(*this);
    }

    return destroyed;
}

void LandmarkObject::setLandmarkColour(const sf::Color& colourA, const sf::Color& colourB)
{
    this->colourA = colourA;
    this->colourB = colourB;
}

const sf::Color& LandmarkObject::getColourA() const
{
    return colourA;
}

const sf::Color& LandmarkObject::getColourB() const
{
    return colourB;
}

BuildableObjectPOD LandmarkObject::getPOD() const
{
    BuildableObjectPOD pod = BuildableObject::getPOD();
    pod.landmarkColourA = colourA;
    pod.landmarkColourB = colourB;
    return pod;
}

void LandmarkObject::loadFromPOD(const BuildableObjectPOD& pod)
{
    BuildableObject::loadFromPOD(pod);
    colourA = pod.landmarkColourA;
    colourB = pod.landmarkColourB;
}