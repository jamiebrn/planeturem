#pragma once

#include <cmath>
#include <vector>

#include <Graphics/VertexArray.hpp>
#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"

class Player;

namespace HealthGUI
{

static constexpr int HEART_SPACING = 10;
static constexpr int HEART_Y_PADDING = 15;
static constexpr int HEART_X_PADDING = 15;
static constexpr int HEART_SIZE = 16;

static constexpr int HEALTH_PER_HEART = 50;

void drawHealth(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Player& player, float gameTime, const std::vector<std::string>& extraInfo);

void drawDeadPrompt(pl::RenderTarget& window);

}