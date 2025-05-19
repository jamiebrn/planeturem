#include "Object/LandmarkObject.hpp"
#include "Game.hpp"

LandmarkObject::LandmarkObject(pl::Vector2f position, ObjectType objectType, Game& game, bool placedByThisPlayer)
    : BuildableObject(position, objectType)
{
    game.landmarkPlaced(*this, placedByThisPlayer);
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

    std::vector<float> replaceKeys = {1.0f, 1.0f, 1.0f, 1.0f, 0, 0, 0, 1.0f};
    std::vector<float> replaceValues = {colorA.r / 255.0f, colorA.g / 255.0f, colorA.b / 255.0f, 1.0f, colorB.r / 255.0f, colorB.g / 255.0f, colorB.b / 255.0f, 1.0f};

    pl::Shader* replaceColorShader = Shaders::getShader(ShaderType::ReplaceColour);
    replaceColorShader->setUniform1i("replaceKeyCount", replaceKeys.size() / 4);
    replaceColorShader->setUniform4fv("replaceKeys", replaceKeys);
    replaceColorShader->setUniform4fv("replaceValues", replaceValues);

    pl::VertexArray rect;
    rect.addQuad(pl::Rect<int>(0, coloredTexture.getHeight(), coloredTexture.getWidth(), -coloredTexture.getHeight()), pl::Color(255, 255, 255, 255), textureRect);

    coloredTexture.draw(rect, *replaceColorShader, TextureManager::getTexture(TextureType::Objects), pl::BlendMode::Alpha);

    std::vector<pl::Rect<int>> textureRectOverride = {pl::Rect<int>(0, 0, coloredTexture.getWidth(), coloredTexture.getHeight())};

    drawObject(window, spriteBatch, camera, gameTime, worldSize, color, textureRectOverride, std::nullopt, &coloredTexture.getTexture());

    // End batch as texture will be freed
    spriteBatch.endDrawing(window);
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

void LandmarkObject::setLandmarkColour(const pl::Color& colorA, const pl::Color& colorB)
{
    this->colorA = colorA;
    this->colorB = colorB;
}

const pl::Color& LandmarkObject::getColorA() const
{
    return colorA;
}

const pl::Color& LandmarkObject::getColorB() const
{
    return colorB;
}

BuildableObjectPOD LandmarkObject::getPOD() const
{
    BuildableObjectPOD pod = BuildableObject::getPOD();
    pod.landmarkColorA = colorA;
    pod.landmarkColorB = colorB;
    return pod;
}

void LandmarkObject::loadFromPOD(const BuildableObjectPOD& pod)
{
    BuildableObject::loadFromPOD(pod);
    colorA = pod.landmarkColorA.value();
    colorB = pod.landmarkColorB.value();
}