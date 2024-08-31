#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <array>
#include <vector>

#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/Sounds.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Helper.hpp"

#include "Player/InventoryData.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/RecipeData.hpp"
#include "Data/RecipeDataLoader.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"

#include "World/ChestDataPool.hpp"

#include "GUI/ItemSlot.hpp"

class InventoryGUI
{
    InventoryGUI() = delete;

public:
    // Initialise animations etc
    static void initialise(InventoryData& inventory);

    static void updateInventory(sf::Vector2f mouseScreenPos, float dt, InventoryData* chestData = nullptr);

    // May pick up item stack, may put down item stack
    static void handleLeftClick(sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData* chestData = nullptr);

    // May pick up single item
    static void handleRightClick(sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData* chestData = nullptr);

    static bool handleScroll(sf::Vector2f mouseScreenPos, int direction);

    // Pickup max stack size to pickup whole stack (may be less than whole stack)
    // Pickup less (e.g 1) to limit pickup amount
    static void pickUpItem(sf::Vector2f mouseScreenPos, unsigned int amount, InventoryData& inventory, InventoryData* chestData = nullptr);

    // Put down whole stack held in cursor
    static void putDownItem(sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData* chestData = nullptr);

    // Called when inventory is closed, handles item picked up (if any)
    static void handleClose(InventoryData& inventory, InventoryData* chestData = nullptr);

    static bool isMouseOverUI(sf::Vector2f mouseScreenPos);

    static void updateAvailableRecipes(InventoryData& inventory, std::unordered_map<std::string, int> nearbyCraftingStationLevels);

    // Gets type of object that will be placed from item currently picked up
    static ObjectType getHeldObjectType();

    // Get type of tool currently picked up from inventory
    static ToolType getHeldToolType();

    // Subtracts from currently held item, called when object is placed
    static void placeHeldObject();

    static bool heldItemPlacesLand();

    static void draw(sf::RenderWindow& window, sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData* chestData = nullptr);

    static inline const std::vector<int>& getAvailableRecipes() {return availableRecipes;}

    // -- Hotbar -- //

    static void updateHotbar(float dt, sf::Vector2f mouseScreenPos);

    static bool handleLeftClickHotbar(sf::Vector2f mouseScreenPos);

    static void handleScrollHotbar(int direction);

    static ObjectType getHotbarSelectedObject(InventoryData& inventory);
    static ToolType getHotbarSelectedTool(InventoryData& inventory);
    static bool hotbarItemPlacesLand(InventoryData& inventory);

    static void placeHotbarObject(InventoryData& inventory);

    // Hotbar drawn when not in inventory
    static void drawHotbar(sf::RenderWindow& window, sf::Vector2f mouseScreenPos, InventoryData& inventory);

    // -- Chest -- //
    static void chestOpened(InventoryData* chestData);

private:
    static void initialiseInventory(InventoryData& inventory);
    static void initialiseHotbar();
    static void createRecipeItemSlots(InventoryData& inventory);
    static void createChestItemSlots(InventoryData* chestData);

    // Returns -1 if no index selected (mouse not hovered over item)
    static int getInventoryHoveredIndex(sf::Vector2f mouseScreenPos);

    // Returns -1 if not hovering over recipe
    static int getHoveredRecipe(sf::Vector2f mouseScreenPos);

    static int getHoveredChestIndex(sf::Vector2f mouseScreenPos);

    static int getHotbarHoveredIndex(sf::Vector2f mouseScreenPos);

    static bool isBinSelected(sf::Vector2f mouseScreenPos);
    static bool isInventorySelected(sf::Vector2f mouseScreenPos);
    static bool isCraftingSelected(sf::Vector2f mouseScreenPos);

    // Attempt to craft recipe selected
    static void craftSelectedRecipe(InventoryData& inventory);

    static void drawItemInfoBox(sf::RenderWindow& window, int itemIndex, InventoryData& inventory, sf::Vector2f mouseScreenPos);
    static void drawItemInfoBoxRecipe(sf::RenderWindow& window, int recipeIdx, sf::Vector2f mouseScreenPos);

    // Position refers to top left of item box
    // static void drawItemBox(sf::RenderWindow& window,
    //                         sf::Vector2f position,
    //                         std::optional<ItemType> itemType = std::nullopt,
    //                         std::optional<int> itemAmount = std::nullopt,
    //                         bool hiddenBackground = false,
    //                         bool selectHighlight = false,
    //                         float itemScaleMult = 1.0f);

    // -- Hotbar --
    static void handleHotbarItemChange();

    // -- Chest --
    // static void addItemToChestAtIndex(int index, ItemType item, int amount, InventoryData* chestData);
    // static void takeItemFromChestAtIndex(int index, int amount, InventoryData* chestData);

private:
    // static sf::Vector2f screenPos;

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

    // Item Slots (visual / interacting)
    static std::vector<ItemSlot> inventoryItemSlots;
    static std::array<ItemSlot, ITEM_BOX_PER_ROW> hotbarItemSlots;
    static std::vector<ItemSlot> recipeItemSlots;
    static std::vector<ItemSlot> chestItemSlots;

    // Index of selected recipe in available recipes
    static int selectedRecipe;

    // Hotbar
    static int selectedHotbarIndex;

    // Chest 
    static constexpr int CHEST_BOX_PER_ROW = 6;

    // Animation
    static AnimatedTexture binAnimation;
    static float binScale;
    static constexpr float BIN_HOVERED_SCALE = 1.2f;
    static constexpr float BIN_HOVERED_SCALE_LERP_WEIGHT = 15.0f;

    // static std::array<float, MAX_INVENTORY_SIZE> inventoryItemScales;

    // static std::vector<float> chestItemScales;

    // static std::array<float, ITEM_BOX_PER_ROW> hotbarItemScales;
    // static constexpr float HOTBAR_SELECTED_SCALE = 1.3f;

    static float hotbarItemStringTimer;
    static constexpr float HOTBAR_ITEM_STRING_OPAQUE_TIME = 2.5f;
    static constexpr float HOTBAR_ITEM_STRING_FADE_TIME = 0.6f;

};