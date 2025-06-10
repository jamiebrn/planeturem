#pragma once

#include <string>
#include <optional>

#include <extlib/steam/steam_api.h>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Framebuffer.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/TextDraw.hpp"
#include "Core/Camera.hpp"
#include "Core/InputManager.hpp"

#include "IO/GameSaveIO.hpp"

#include "World/WorldData.hpp"
#include "World/ChunkManager.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/DefaultGUIPanel.hpp"

#include "GameConstants.hpp"

class DemoEndGUI : public DefaultGUIPanel
{
public:
    DemoEndGUI() = default;

    void initialise();

    bool createAndDraw(pl::RenderTarget& window, float dt);

};