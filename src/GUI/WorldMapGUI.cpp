#include "GUI/WorldMapGUI.hpp"

void WorldMapGUI::drawMiniMap(pl::RenderTarget& window, const WorldMap& worldMap, pl::Vector2f playerPosition, ObjectReference spawnLocation)
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
    const pl::Rect<int> spawnLocationIconTextureRect(160, 64, 6, 6);
    pl::Vector2f spawnLocationMap = static_cast<pl::Vector2f>(spawnLocation.getWorldTile()) / (CHUNK_TILE_SIZE / CHUNK_MAP_TILE_SIZE);
    miniMapRect.clear();
    miniMapRect.addQuad(pl::Rect<float>(pl::Vector2f(-playerPositionMap.x * MINI_MAP_SCALE + MINI_MAP_WIDTH / 2 + spawnLocationMap.x * MINI_MAP_SCALE,
        -playerPositionMap.y * MINI_MAP_SCALE + MINI_MAP_HEIGHT / 2 + spawnLocationMap.y * MINI_MAP_SCALE) - spawnLocationIconTextureRect.getSize() / 2,
        spawnLocationIconTextureRect.getSize() * MINI_MAP_SCALE), pl::Color(), static_cast<pl::Rect<float>>(spawnLocationIconTextureRect));
    
    miniMapFrameBuffer.draw(miniMapRect, *Shaders::getShader(ShaderType::Default), &worldMap.getTexture(), pl::BlendMode::Alpha);

    // pl::Framebuffer miniMapCutoutFramebuffer;
    // miniMapCutoutFramebuffer.create(MINI_MAP_WIDTH / MINI_MAP_SCALE, MINI_MAP_HEIGHT / MINI_MAP_SCALE);
    // miniMapCutoutFramebuffer.clear(pl::Color(0, 0, 0, 0));

    // miniMapRect.clear();
    // miniMapRect.addQuad(pl::Rect<float>(0, 0, miniMapCutoutFramebuffer.getWidth(), miniMapCutoutFramebuffer.getHeight()), pl::Color(),
    //     pl::Rect<float>(0, 0, miniMapFrameBuffer.getWidth(), miniMapFrameBuffer.getHeight()));
    
    // miniMapCutoutFramebuffer.draw(miniMapRect, *Shaders::getShader(ShaderType::MiniMap), &miniMapFrameBuffer.getTexture(), pl::BlendMode::Alpha);

    Shaders::getShader(ShaderType::MiniMap)->setUniform1i("circleResolution", MINI_MAP_WIDTH / 4);

    miniMapRect.clear();
    miniMapRect.addQuad(pl::Rect<float>(window.getWidth() - (MINI_MAP_WIDTH + MINI_MAP_PADDING) * intScale,
        window.getHeight() - (MINI_MAP_HEIGHT + MINI_MAP_PADDING) * intScale, MINI_MAP_WIDTH * intScale, MINI_MAP_HEIGHT * intScale), pl::Color(),
        pl::Rect<float>(0, miniMapFrameBuffer.getHeight(), miniMapFrameBuffer.getWidth(), -miniMapFrameBuffer.getHeight()));

    window.draw(miniMapRect, *Shaders::getShader(ShaderType::MiniMap), &miniMapFrameBuffer.getTexture(), pl::BlendMode::Alpha);
}

void WorldMapGUI::drawMap(pl::RenderTarget& window, const WorldMap& worldMap, pl::Vector2f playerPosition)
{

}