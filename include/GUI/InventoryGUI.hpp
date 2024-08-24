#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>

#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"

#include "Player/Inventory.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/RecipeData.hpp"
#include "Data/RecipeDataLoader.hpp"

class InventoryGUI
{
    InventoryGUI() = delete;

public:
    // May pick up item stack, may put down item stack
    static void handleLeftClick(sf::Vector2f mouseScreenPos);

    // May pick up single item
    static void handleRightClick(sf::Vector2f mouseScreenPos);

    static bool handleScroll(sf::Vector2f mouseScreenPos, int direction);

    // Pickup max stack size to pickup whole stack (may be less than whole stack)
    // Pickup less (e.g 1) to limit pickup amount
    static void pickUpItem(sf::Vector2f mouseScreenPos, unsigned int amount = INVENTORY_STACK_SIZE);

    // Put down whole stack held in cursor
    static void putDownItem(sf::Vector2f mouseScreenPos);

    // Called when inventory is closed, handles item picked up (if any)
    static void handleClose();

    static bool isMouseOverUI(sf::Vector2f mouseScreenPos);

    static void updateAvailableRecipes(std::unordered_map<std::string, int> nearbyCraftingStationLevels);

    // Gets type of object that will be placed from item currently picked up
    static ObjectType getHeldObjectType();

    // Subtracts from currently held item, called when object is placed
    static void placeHeldObject();

    static void draw(sf::RenderWindow& window, sf::Vector2f mouseScreenPos);

    static inline const std::vector<int>& getAvailableRecipes() {return availableRecipes;}

private:
    // Returns -1 if no index selected (mouse not hovered over item)
    static int getInventorySelectedIndex(sf::Vector2f mouseScreenPos);

    // Returns -1 if not clicked / hovering over recipe
    // Call on left click
    static int getClickedRecipe(sf::Vector2f mouseScreenPos);

    static bool isBinSelected(sf::Vector2f mouseScreenPos);
    static bool isInventorySelected(sf::Vector2f mouseScreenPos);
    static bool isCraftingSelected(sf::Vector2f mouseScreenPos);

    // Attempt to craft recipe selected
    static void craftSelectedRecipe();

    static void drawItemInfoBoxInventory(sf::RenderWindow& window, int itemIndex, sf::Vector2f mouseScreenPos);
    static void drawItemInfoBoxRecipe(sf::RenderWindow& window, int recipeIdx, sf::Vector2f mouseScreenPos);

    // Position refers to top left of item box
    static void drawItemBox(sf::RenderWindow& window,
                            sf::Vector2f position,
                            std::optional<ItemType> itemType = std::nullopt,
                            std::optional<int> itemAmount = std::nullopt,
                            bool hiddenBackground = false,
                            bool selectHighlight = false);

private:
    static sf::Vector2f screenPos;

    static int itemBoxSize;
    static int itemBoxSpacing;
    static int itemBoxPadding;
    static int itemBoxPerRow;

    static bool isItemPickedUp;
    static ItemType pickedUpItem;
    static int pickedUpItemCount;

    static std::unordered_map<std::string, int> previous_nearbyCraftingStationLevels;
    static std::unordered_map<ItemType, unsigned int> previous_inventoryItemCount;
    static std::vector<int> availableRecipes;

    // Index of selected recipe in available recipes
    static int selectedRecipe;

};