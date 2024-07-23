#pragma once

#include <SFML/Graphics.hpp>

#include "Inventory.hpp"
#include "TextureManager.hpp"
#include "TextDraw.hpp"

class InventoryGUI
{
    InventoryGUI() = delete;

public:
    static void draw(sf::RenderWindow& window);

};