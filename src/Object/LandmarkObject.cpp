#include "Object/LandmarkObject.hpp"
#include "Game.hpp"

LandmarkObject::LandmarkObject(pl::Vector2f position, ObjectType objectType, Game& game, bool placedByPlayer)
    : BuildableObject(position, objectType)
{
    game.landmarkPlaced(*this, placedByPlayer);
}

BuildableObject* LandmarkObject::clone()
{
    return new LandmarkObject(*this);
}

void LandmarkObject::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const pl::Color& color) const
{
    const ObjectData& objectData = ObjectDataLoader::getObjectData(objectType);

    pl::Rect<int> textureRect = objectData.textureRects[animatedTexture.getFrame()];

    pl::Framebuffer coloredTexture;
    coloredTexture.create(textureRect.width, textureRect.height);
    coloredTexture.clear(pl::Color(0, 0, 0, 0));

    std::vector<float> replaceKeys = {1.0f, 1.0f, 1.0f, 0, 0, 0, 0, 1.0f};
    std::vector<float> replaceValues = {colorA.r / 255.0f, colorA.g / 255.0f, colorA.b / 255.0f, 1.0f, colorB.r / 255.0f, colorB.g / 255.0f, colorB.b / 255.0f, 1.0f};

    pl::Shader* replaceColorShader = Shaders::getShader(ShaderType::ReplaceColour);
    replaceColourShader->setUniform1i("replaceKeyCount", 2);
    replaceColourShader->setUniform4fv("replaceKeys", replaceKeys);
    replaceColourShader->setUniform4fv("replaceValues", replaceValues);

    pl::VertexArray rect;
    rect.addQuad(pl::Rect<int>(0, 0, coloredTexture.getWidth(), coloredTexture.getHeight()), pl::Color(255, 255, 255, 255), textureRect);

    coloredTexture.draw(rect, *replaceColorShader, TextureManager::getTexture(TextureType::Objects), pl::BlendMode::Alpha);

    drawObject(window, spriteBatch, camera, gameTime, worldSize, color, std::nullopt, std::nullopt, &coloredTexture.getTexture());
}

bool LandmarkObject::damage(int amount, Game& game, ChunkManager& chunkManager, ParticleSystem& particleSystem, bool giveItems)
{
    bool destroyed = BuildableObject::damage(amount, game, chunkManager, particleSystem, giveItems);

    if (destroyed)
    {
        game.landmarkDestroyed(*this);
    }

    return destroyed;
}

void LandmarkObject::interact(Game& game, bool isClient)
{
    game.landmarkPlaced(*this, true);
}

bool LandmarkObject::isInteractable() const
{
    return true;
}

void LandmarkObject::setLandmarkColour(const pl::Color& colourA, const pl::Color& colourB)
{
    this->colourA = colourA;
    this->colourB = colourB;
}

const pl::Color& LandmarkObject::getColourA() const
{
    return colourA;
}

const pl::Color& LandmarkObject::getColourB() const
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
    colourA = pod.landmarkColourA.value();
    colourB = pod.landmarkColourB.value();
}