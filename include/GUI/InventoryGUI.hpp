#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>

#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Helper.hpp"

#include "Player/Inventory.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/RecipeData.hpp"
#include "Data/RecipeDataLoader.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"

class InventoryGUI
{
    InventoryGUI() = delete;

public:
    // Initialise animations etc
    static void initialise();

    static void updateAnimations(sf::Vector2f mouseScreenPos, float dt);

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

    // Get type of tool currently picked up from inventory
    static ToolType getHeldToolType();

    // Subtracts from currently held item, called when object is placed
    static void placeHeldObject();

    static bool heldItemPlacesLand();

    static void draw(sf::RenderWindow& window, sf::Vector2f mouseScreenPos);

    static inline const std::vector<int>& getAvailableRecipes() {return availableRecipes;}

    // -- Hotbar -- //

    static void updateAnimationsHotbar(float dt);

    static void handleScrollHotbar(int direction);

    static ObjectType getHotbarSelectedObject();
    static ToolType getHotbarSelectedTool();
    static bool hotbarItemPlacesLand();

    static void placeHotbarObject();

    // Hotbar drawn when not in inventory
    static void drawHotbar(sf::RenderWindow& window, sf::Vector2f mouseScreenPos);

private:
    // Returns -1 if no index selected (mouse not hovered over item)
    static int getInventoryHoveredIndex(sf::Vector2f mouseScreenPos);

    // Returns -1 if not hovering over recipe
    static int getHoveredRecipe(sf::Vector2f mouseScreenPos);

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
                            bool selectHighlight = false,
                            float itemScaleMult = 1.0f);

private:
    static sf::Vector2f screenPos;

    static int itemBoxSize;
    static int itemBoxSpacing;
    static int itemBoxPadding;
    static constexpr int ITEM_BOX_PER_ROW = 8;

    static bool isItemPickedUp;
    static ItemType pickedUpItem;
    static int pickedUpItemCount;

    static std::unordered_map<std::string, int> previous_nearbyCraftingStationLevels;
    static std::unordered_map<ItemType, unsigned int> previous_inventoryItemCount;
    static std::vector<int> availableRecipes;

    // Index of selected recipe in available recipes
    static int selectedRecipe;

    // Hotbar
    static int selectedHotbarIndex;

    // Animation
    static AnimatedTexture binAnimation;
    static float binScale;
    static constexpr float BIN_HOVERED_SCALE = 1.2f;

    static std::array<float, MAX_INVENTORY_SIZE> inventoryItemScales;
    static constexpr float ITEM_HOVERED_SCALE = 1.3f;
    static constexpr float ITEM_HOVERED_SCALE_LERP_WEIGHT = 15.0f;

    static std::array<float, ITEM_BOX_PER_ROW> hotbarItemScales;
    static constexpr float HOTBAR_SELECTED_SCALE = 1.3f;

};