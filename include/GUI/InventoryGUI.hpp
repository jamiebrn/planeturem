#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <vector>
#include <optional>
#include <cmath>
#include <stack>

#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/Sounds.hpp"
#include "Core/AnimatedTexture.hpp"
#include "Core/Helper.hpp"
#include "Core/SpriteBatch.hpp"
#include "Core/InputManager.hpp"

#include "Player/PlayerStats.hpp"
#include "Player/InventoryData.hpp"
#include "Player/ShopInventoryData.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/RecipeData.hpp"
#include "Data/RecipeDataLoader.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"
#include "Data/ArmourData.hpp"
#include "Data/ArmourDataLoader.hpp"
#include "Data/StructureData.hpp"
#include "Data/StructureDataLoader.hpp"
#include "Data/PlanetGenData.hpp"
#include "Data/PlanetGenDataLoader.hpp"

#include "World/ChestDataPool.hpp"

#include "GUI/ItemSlot.hpp"

// Forward declare
class Game;

struct ItemInfoString
{
    std::string string;
    unsigned int size = 24;
    sf::Color color = sf::Color(255, 255, 255);

    std::optional<ItemCount> itemCount = std::nullopt;

    bool drawItemCountNumberWhenOne = true;
};

struct ItemPopup
{
    ItemCount itemCount;
    float timeAlive = 0;
    bool notEnoughSpace = false;
    std::optional<std::string> textOverride;
};

enum class InventoryShopInfoMode
{
    None,
    Buy,
    Sell
};

class InventoryGUI
{
    InventoryGUI() = delete;

public:
    // Initialise animations etc
    static void initialise(InventoryData& inventory);

    static void reset();

    static void updateInventory(sf::Vector2f mouseScreenPos, float dt, InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData = nullptr);

    // May pick up item stack, may put down item stack
    static void handleLeftClick(Game& game, sf::Vector2f mouseScreenPos, bool shiftMode, InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData = nullptr);

    // May pick up single item
    static void handleRightClick(Game& game, sf::Vector2f mouseScreenPos, bool shiftMode, InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData = nullptr);

    static bool handleScroll(sf::Vector2f mouseScreenPos, int direction, InventoryData& inventory);

    // Pickup max stack size to pickup whole stack (may be less than whole stack)
    // Pickup less (e.g 1) to limit pickup amount
    static void pickUpItem(Game& game, sf::Vector2f mouseScreenPos, unsigned int amount, InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData = nullptr);

    // Put down whole stack held in cursor
    static void putDownItem(Game& game, sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData = nullptr);

    // Called when inventory is closed, handles item picked up (if any)
    static void handleClose(InventoryData& inventory, InventoryData* chestData = nullptr);

    static bool isMouseOverUI(sf::Vector2f mouseScreenPos);

    static void updateAvailableRecipes(InventoryData& inventory, std::unordered_map<std::string, int> nearbyCraftingStationLevels);
    static void setSeenRecipes(const std::unordered_set<ItemType>& recipes);
    static const std::unordered_set<ItemType>& getSeenRecipes();

    static ItemType getHeldItemType(InventoryData& inventory);

    // Gets type of object that will be placed from item currently picked up / selected in hotbar
    // INCLUDES HOTBAR AND ITEM PICKED UP
    static ObjectType getHeldObjectType(InventoryData& inventory);

    // Get type of tool currently picked up from inventory
    // INCLUDES HOTBAR AND ITEM PICKED UP
    static ToolType getHeldToolType(InventoryData& inventory);

    // Subtracts from currently held item, called when object is placed etc
    // Subtracts from item picked up, if not will attempt to place from hotbar
    static void subtractHeldItem(InventoryData& inventory);

    static bool heldItemPlacesLand(InventoryData& inventory);

    static void draw(sf::RenderTarget& window, float gameTime, sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData& armourInventory,
        InventoryData* chestData = nullptr);

    static inline const std::vector<int>& getAvailableRecipes() {return availableRecipes;}

    // -- Hotbar -- //

    static void updateHotbar(float dt, sf::Vector2f mouseScreenPos);

    static bool handleLeftClickHotbar(sf::Vector2f mouseScreenPos);

    static void handleScrollHotbar(int direction);
    static void setHotbarSelectedIndex(int index);

    static ObjectType getHotbarSelectedObject(InventoryData& inventory);
    static ToolType getHotbarSelectedTool(InventoryData& inventory);
    static bool hotbarItemPlacesLand(InventoryData& inventory);

    static void subtractHotbarItem(InventoryData& inventory);

    // Hotbar drawn when not in inventory
    static void drawHotbar(sf::RenderTarget& window, sf::Vector2f mouseScreenPos, InventoryData& inventory);

    // -- Chest -- //
    static void chestOpened(InventoryData* chestData);
    static void chestClosed();

    // -- Shop -- //
    static void shopOpened(ShopInventoryData& shopData);
    static void shopClosed();

    // -- Popups -- //
    static void updateItemPopups(float dt);

    static void pushItemPopup(const ItemCount& itemCount, bool notEnoughSpace = false, std::optional<std::string> textOverride = std::nullopt);

    static void drawItemPopups(sf::RenderTarget& window, float gameTime);

    // -- Controller navigation -- //

    // Returns true if any inventory is modified
    static bool handleControllerInput(Game& game, InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData);

    // -- Misc -- //
    static bool canQuickTransfer(sf::Vector2f mouseScreenPos, bool shiftMode, InventoryData& inventory, InventoryData* chestData);

private:
    static void initialiseInventory(InventoryData& inventory);
    static void initialiseHotbar();
    static void createRecipeItemSlots(InventoryData& inventory);
    static void createChestItemSlots(InventoryData* chestData);

    // Returns -1 if no index selected (mouse not hovered over item)
    static int getHoveredItemSlotIndex(const std::vector<ItemSlot>& itemSlots, sf::Vector2f mouseScreenPos);

    static bool isBinSelected(sf::Vector2f mouseScreenPos);
    static bool isInventorySelected(sf::Vector2f mouseScreenPos);
    static bool isCraftingSelected(sf::Vector2f mouseScreenPos);

    // Attempt to craft recipe selected
    static void craftRecipe(InventoryData& inventory, int selectedRecipe);

    static void drawInventory(sf::RenderTarget& window, InventoryData& inventory);
    static void drawArmourInventory(sf::RenderTarget& window, InventoryData& armourInventory);
    static void drawBin(sf::RenderTarget& window);
    static void drawRecipes(sf::RenderTarget& window);
    static void drawChest(sf::RenderTarget& window, InventoryData* chestData);
    static void drawPickedUpItem(sf::RenderTarget& window, float gameTime, sf::Vector2f mouseScreenPos);
    static void drawHoveredItemInfoBox(sf::RenderTarget& window, float gameTime, sf::Vector2f mouseScreenPos, InventoryData& inventory,
        InventoryData& armourInventory, InventoryData* chestData);

    static sf::Vector2f drawItemInfoBox(sf::RenderTarget& window, float gameTime, int itemIndex, InventoryData& inventory, sf::Vector2f mouseScreenPos,
        InventoryShopInfoMode shopInfoMode);
    static sf::Vector2f drawItemInfoBox(sf::RenderTarget& window, float gameTime, ItemCount itemCount, sf::Vector2f mouseScreenPos,
        InventoryShopInfoMode shopInfoMode);
    static sf::Vector2f drawItemInfoBoxRecipe(sf::RenderTarget& window, float gameTime, int recipeIdx, sf::Vector2f mouseScreenPos);

    // Returns size of drawn info box
    static sf::Vector2f drawInfoBox(sf::RenderTarget& window, sf::Vector2f position, const std::vector<ItemInfoString>& infoStrings, int alpha = 255, float flashAmount = 0.0f);

    // Armour inventory helper
    static bool canPutDownItemInArmourInventory(int hoveredIndex);

    // -- Hotbar --
    static void handleHotbarItemChange();

    // -- Chest --
    static void inventoryChestItemQuickTransfer(Game& game, sf::Vector2f mouseScreenPos, unsigned int amount, InventoryData& inventory, InventoryData& chestData);

    // -- Shop --
    static bool attemptPurchaseItem(InventoryData& inventory, int shopIndex);
    static bool attemptSellItemHeld(InventoryData& inventory);

private:
    // static sf::Vector2f screenPos;

    static int itemBoxSize;
    static int itemBoxSpacing;
    static int itemBoxPadding;
    static constexpr int ITEM_BOX_PER_ROW = 8;

    static constexpr int ARMOUR_SLOTS = 3;

    static bool isItemPickedUp;
    static ItemType pickedUpItem;
    static int pickedUpItemCount;

    static std::unordered_map<std::string, int> previous_nearbyCraftingStationLevels;
    static std::unordered_map<ItemType, unsigned int> previous_inventoryItemCount;
    static std::vector<int> availableRecipes;
    static std::unordered_set<ItemType> recipesSeen;
    static std::stack<ItemType> recipesSeenToNotify;
    static constexpr float MAX_RECIPE_SEEN_NOTIFY_COOLDOWN = 0.2f;
    static float recipeSeenNotifyCooldown;

    // Item Slots (visual / interacting)
    static std::vector<ItemSlot> inventoryItemSlots;
    static std::vector<ItemSlot> armourItemSlots;
    static std::vector<ItemSlot> hotbarItemSlots;
    static std::vector<ItemSlot> recipeItemSlots;
    static std::vector<ItemSlot> chestItemSlots;
    static std::vector<ItemSlot> binItemSlot;

    // Index of selected recipe in available recipes
    // static int selectedRecipe;
    static int recipeCurrentPage;
    static constexpr int RECIPE_MAX_ROWS = 3;

    // Hotbar
    static int selectedHotbarIndex;

    // Chest 
    static constexpr int CHEST_BOX_PER_ROW = 6;
    static int currentChestBoxPerRow;

    // Shop
    static std::optional<ShopInventoryData> openShopData;

    // Item pop-up
    static std::vector<ItemPopup> itemPopups;
    static constexpr float POPUP_LIFETIME = 7.0f;
    static constexpr float POPUP_FADE_TIME = 0.8f;
    static constexpr float POPUP_FLASH_TIME = 0.4f;
    static constexpr int POPUP_MAX_COUNT = 5;

    // Animation
    static AnimatedTexture binAnimation;
    static float binScale;
    static constexpr float BIN_HOVERED_SCALE = 1.2f;
    static constexpr float BIN_HOVERED_SCALE_LERP_WEIGHT = 15.0f;

    static float hotbarItemStringTimer;
    static constexpr float HOTBAR_ITEM_STRING_OPAQUE_TIME = 2.5f;
    static constexpr float HOTBAR_ITEM_STRING_FADE_TIME = 0.6f;

    // Controller navigation
    static int controllerSelectedSlotIndex;
    static std::vector<ItemSlot>* controllerSelectedItemSlots;

};