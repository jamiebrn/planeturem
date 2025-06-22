#pragma once

#include <vector>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "GameConstants.hpp"

#include "Core/ResolutionHandler.hpp"
#include "Core/Shaders.hpp"
#include "Core/TextureManager.hpp"
#include "Core/Camera.hpp"

#include "World/WorldMap.hpp"
#include "Object/ObjectReference.hpp"

class WorldMapGUI
{
public:
    WorldMapGUI() = default;

    void drawMiniMap(pl::RenderTarget& window, const WorldMap& worldMap, pl::Vector2f playerPosition, ObjectReference spawnLocation);

    void drawMap(pl::RenderTarget& window, const WorldMap& worldMap, pl::Vector2f playerPosition);

private:


};