#pragma once

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"

void drawUIKeyboardButton(sf::RenderTarget& window, std::string keyString, sf::Vector2f position, float scale, bool centred = true);

void drawUIMouseScrollWheelDirection(sf::RenderTarget& window, int direction, sf::Vector2f position, float scale, bool centred = true);