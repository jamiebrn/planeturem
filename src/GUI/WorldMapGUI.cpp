#include "GUI/WorldMapGUI.hpp"

void WorldMapGUI::drawMiniMap(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, float gameTime, const WorldMap& worldMap,
    pl::Vector2f playerPosition, ObjectReference spawnLocation, const std::vector<LandmarkSummaryData>& landmarkSummaryData)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    static constexpr int MINI_MAP_PADDING = 30;
    static constexpr int MINI_MAP_WIDTH = 300;
    static constexpr int MINI_MAP_HEIGHT = 300;
    static constexpr int MINI_MAP_SCALE = 3;

    pl::Framebuffer miniMapFrameBuffer;
    miniMapFrameBuffer.create(MINI_MAP_WIDTH, MINI_MAP_HEIGHT);
    miniMapFrameBuffer.clear(pl::Color(0, 0, 0));

    // Get player position in world map coordinates
    pl::Vector2f playerPositionMap = playerPosition / TILE_SIZE_PIXELS_UNSCALED / (CHUNK_TILE_SIZE / CHUNK_MAP_TILE_SIZE);

    pl::VertexArray miniMapRect;
    miniMapRect.addQuad(pl::Rect<float>(-playerPositionMap.x * MINI_MAP_SCALE + MINI_MAP_WIDTH / 2 -
        worldMap.getTexture().getWidth() * MINI_MAP_SCALE, -playerPositionMap.y * MINI_MAP_SCALE + MINI_MAP_HEIGHT / 2 -
        worldMap.getTexture().getHeight() * MINI_MAP_SCALE,
        worldMap.getTexture().getWidth() * MINI_MAP_SCALE * 3, worldMap.getTexture().getHeight() * MINI_MAP_SCALE * 3),
        pl::Color(), pl::Rect<float>(0, 0, worldMap.getTexture().getWidth() * 3, worldMap.getTexture().getHeight() * 3));

    miniMapFrameBuffer.draw(miniMapRect, *Shaders::getShader(ShaderType::Default), &worldMap.getTexture(), pl::BlendMode::Alpha);

    // Draw spawn location
    const pl::Rect<int> spawnLocationIconTextureRect(160, 64, 8, 7);

    pl::Vector2f spawnLocationWorld = spawnLocation.getWorldTile() * TILE_SIZE_PIXELS_UNSCALED;
    spawnLocationWorld = Camera::translateWorldPos(spawnLocationWorld, playerPosition, worldMap.getWorldSize());

    pl::Vector2f spawnLocationMap = spawnLocationWorld / TILE_SIZE_PIXELS_UNSCALED / (CHUNK_TILE_SIZE / CHUNK_MAP_TILE_SIZE);
    miniMapRect.clear();
    miniMapRect.addQuad(pl::Rect<float>(pl::Vector2f(-playerPositionMap.x * MINI_MAP_SCALE + MINI_MAP_WIDTH / 2 + spawnLocationMap.x * MINI_MAP_SCALE,
        -playerPositionMap.y * MINI_MAP_SCALE + MINI_MAP_HEIGHT / 2 + spawnLocationMap.y * MINI_MAP_SCALE) - spawnLocationIconTextureRect.getSize() / 2 * MINI_MAP_SCALE,
        spawnLocationIconTextureRect.getSize() * MINI_MAP_SCALE), pl::Color(), static_cast<pl::Rect<float>>(spawnLocationIconTextureRect));
    
    // Draw player location
    const pl::Rect<int> playerIconTextureRect(160, 77, 3, 3);

    miniMapRect.addQuad(pl::Rect<float>(pl::Vector2f(MINI_MAP_WIDTH / 2, MINI_MAP_HEIGHT / 2) - playerIconTextureRect.getSize() / 2 * MINI_MAP_SCALE,
        playerIconTextureRect.getSize() * MINI_MAP_SCALE), pl::Color(), static_cast<pl::Rect<float>>(playerIconTextureRect));

    miniMapFrameBuffer.draw(miniMapRect, *Shaders::getShader(ShaderType::Default), TextureManager::getTexture(TextureType::UI), pl::BlendMode::Alpha);

    Shaders::getShader(ShaderType::MiniMap)->setUniform1i("circleResolution", MINI_MAP_WIDTH / 4);

    miniMapRect.clear();
    miniMapRect.addQuad(pl::Rect<float>(window.getWidth() - (MINI_MAP_WIDTH + MINI_MAP_PADDING) * intScale,
        window.getHeight() - (MINI_MAP_HEIGHT + MINI_MAP_PADDING) * intScale, MINI_MAP_WIDTH * intScale, MINI_MAP_HEIGHT * intScale), pl::Color(),
        pl::Rect<float>(0, miniMapFrameBuffer.getHeight(), miniMapFrameBuffer.getWidth(), -miniMapFrameBuffer.getHeight()));

    window.draw(miniMapRect, *Shaders::getShader(ShaderType::MiniMap), &miniMapFrameBuffer.getTexture(), pl::BlendMode::Alpha);

    // Draw landmarks
    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::UI);
    drawData.shader = Shaders::getShader(ShaderType::ReplaceColour);
    drawData.scale = pl::Vector2f(3, 3) * intScale;
    drawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    drawData.color = pl::Color(255, 255, 255, 150);
    
    std::vector<float> replaceKeys = {1, 1, 1, 1, 0, 0, 0, 1};
    drawData.shader->setUniform1i("replaceKeyCount", replaceKeys.size() / 4);
    drawData.shader->setUniform4fv("replaceKeys", replaceKeys);

    AnimatedTexture landmarkUIAnimation(6, 16, 16, 96, 112, 0.1);

    landmarkUIAnimation.setFrame(static_cast<int>(gameTime / 0.1) % 6);

    for (const LandmarkSummaryData& landmarkSummary : landmarkSummaryData)
    {
        pl::Color colorANormalised = landmarkSummary.colorA.normalise();
        pl::Color colorBNormalised = landmarkSummary.colorB.normalise();
        
        std::vector<float> replaceValues = {
            colorANormalised.r, colorANormalised.g, colorANormalised.b, colorANormalised.a,
            colorBNormalised.r, colorBNormalised.g, colorBNormalised.b, colorBNormalised.a
        };

        pl::Vector2f worldPos = Camera::translateWorldPos(landmarkSummary.worldPos, playerPosition, worldMap.getWorldSize());

        pl::Vector2f relativePos = (worldPos - playerPosition).normalise() *
            std::min((worldPos - playerPosition).getLength() / TILE_SIZE_PIXELS_UNSCALED / (CHUNK_TILE_SIZE / CHUNK_MAP_TILE_SIZE) * MINI_MAP_SCALE * intScale,
            MINI_MAP_WIDTH * intScale / 2);

        drawData.position = relativePos + pl::Vector2f(window.getWidth() - (MINI_MAP_WIDTH / 2 + MINI_MAP_PADDING) * intScale,
            window.getHeight() - (MINI_MAP_HEIGHT / 2 + MINI_MAP_PADDING) * intScale);
        drawData.shader->setUniform4fv("replaceValues", replaceValues);
        drawData.textureRect = landmarkUIAnimation.getTextureRect();
    
        spriteBatch.draw(window, drawData);
    }
}

void WorldMapGUI::drawMap(pl::RenderTarget& window, const WorldMap& worldMap, pl::Vector2f playerPosition)
{

}