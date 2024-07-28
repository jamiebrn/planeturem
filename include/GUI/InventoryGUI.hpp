#pragma once

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Player/Inventory.hpp"
#include "Data/ItemDataLoader.hpp"

class InventoryGUI
{
    InventoryGUI() = delete;

public:
    static void draw(sf::RenderWindow& window);

};