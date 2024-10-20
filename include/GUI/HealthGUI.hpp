#pragma once

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/Shaders.hpp"
#include "Core/ResolutionHandler.hpp"

namespace HealthGUI
{

static constexpr int HEART_SPACING = 10;
static constexpr int HEART_Y_PADDING = 15;
static constexpr int HEART_X_PADDING = 15;
static constexpr int HEART_SIZE = 16;

void drawHealth(sf::RenderTarget& window, SpriteBatch& spriteBatch, int health, int maxHealth);

}