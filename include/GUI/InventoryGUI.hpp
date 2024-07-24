#pragma once

#include <SFML/Graphics.hpp>

#include "Player/Inventory.hpp"
#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"

class InventoryGUI
{
    InventoryGUI() = delete;

public:
    static void draw(sf::RenderWindow& window);

};