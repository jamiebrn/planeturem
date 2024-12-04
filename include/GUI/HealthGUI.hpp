#pragma once

#include <cmath>
#include <vector>

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/SpriteBatch.hpp"
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

void drawHealth(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Player& player, float gameTime, const std::vector<std::string>& extraInfo);

void drawDeadPrompt(sf::RenderTarget& window);

}