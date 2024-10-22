#pragma once

#include <cmath>

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/Shaders.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"

namespace HealthGUI
{

static constexpr int HEART_SPACING = 10;
static constexpr int HEART_Y_PADDING = 15;
static constexpr int HEART_X_PADDING = 15;
static constexpr int HEART_SIZE = 16;

static constexpr int HEALTH_PER_HEART = 50;

void drawHealth(sf::RenderTarget& window, SpriteBatch& spriteBatch, int health, int maxHealth, float gameTime);

void drawDeadPrompt(sf::RenderTarget& window);

}