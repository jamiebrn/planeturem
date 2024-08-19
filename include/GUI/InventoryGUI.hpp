#pragma once

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"

#include "Player/Inventory.hpp"
#include "Data/ItemDataLoader.hpp"

class InventoryGUI
{
    InventoryGUI() = delete;

public:
    // May pick up item stack, may put down item stack
    static void handleLeftClick(sf::Vector2f mouseScreenPos);

    // May pick up single item
    static void handleRightClick(sf::Vector2f mouseScreenPos);

    // Pickup max stack size to pickup whole stack (may be less than whole stack)
    // Pickup less (e.g 1) to limit pickup amount
    static void pickUpItem(sf::Vector2f mouseScreenPos, unsigned int amount = INVENTORY_STACK_SIZE);

    // Put down whole stack held in cursor
    static void putDownItem(sf::Vector2f mouseScreenPos);

    // Called when inventory is closed, handles item picked up (if any)
    static void handleClose();

    static bool isMouseOverUI(sf::Vector2f mouseScreenPos);

    static void draw(sf::RenderWindow& window);

private:
    // Returns -1 if no index selected (mouse not hovered over item)
    static int getInventorySelectedIndex(sf::Vector2f mouseScreenPos);

private:
    static sf::Vector2f screenPos;

    static int itemBoxSize;
    static int itemBoxSpacing;
    static int itemBoxPadding;
    static int itemBoxPerRow;

    static bool isItemPickedUp;
    static ItemType pickedUpItem;
    static int pickedUpItemCount;

};