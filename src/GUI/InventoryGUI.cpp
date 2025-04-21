#include "GUI/InventoryGUI.hpp"
#include "Game.hpp"
#include "Network/NetworkHandler.hpp"

int InventoryGUI::itemBoxSize = 75;
int InventoryGUI::itemBoxSpacing = 10;
int InventoryGUI::itemBoxPadding = 10;

bool InventoryGUI::isItemPickedUp = false;
ItemType InventoryGUI::pickedUpItem;
int InventoryGUI::pickedUpItemCount;

std::unordered_map<std::string, int> InventoryGUI::previous_nearbyCraftingStationLevels;
std::unordered_map<ItemType, unsigned int> InventoryGUI::previous_inventoryItemCount;
std::vector<int> InventoryGUI::availableRecipes;
std::unordered_set<ItemType> InventoryGUI::recipesSeen;
std::stack<ItemType> InventoryGUI::recipesSeenToNotify;
float InventoryGUI::recipeSeenNotifyCooldown = 0.0f;

std::vector<ItemSlot> InventoryGUI::inventoryItemSlots;
std::vector<ItemSlot> InventoryGUI::armourItemSlots;
std::vector<ItemSlot> InventoryGUI::hotbarItemSlots;
std::vector<ItemSlot> InventoryGUI::recipeItemSlots;
std::vector<ItemSlot> InventoryGUI::chestItemSlots;
std::vector<ItemSlot> InventoryGUI::binItemSlot;

// int InventoryGUI::selectedRecipe = 0;
int InventoryGUI::recipeCurrentPage = 0;

int InventoryGUI::selectedHotbarIndex = 0;

int InventoryGUI::currentChestBoxPerRow = CHEST_BOX_PER_ROW;

std::optional<ShopInventoryData> InventoryGUI::openShopData;

std::vector<ItemPopup> InventoryGUI::itemPopups;

AnimatedTexture InventoryGUI::binAnimation;
float InventoryGUI::binScale = 1.0f;

float InventoryGUI::hotbarItemStringTimer = 0.0f;

int InventoryGUI::controllerSelectedSlotIndex = 0;
std::vector<ItemSlot>* InventoryGUI::controllerSelectedItemSlots = nullptr;

void InventoryGUI::initialise(InventoryData& inventory)
{
    binAnimation.create(4, 16, 20, 96, 12, 0.04f, false);

    // inventoryItemScales.fill(1.0f);

    // hotbarItemScales.fill(1.0f);

    initialiseInventory(inventory);
    initialiseHotbar();
}

void InventoryGUI::initialiseInventory(InventoryData& inventory)
{
    pl::Vector2f itemBoxPosition = pl::Vector2f(itemBoxPadding, itemBoxPadding);

    int currentRowIndex = 0;

    for (int itemIndex = 0; itemIndex < inventory.getSize(); itemIndex++)
    {
        inventoryItemSlots.push_back(ItemSlot(itemBoxPosition, itemBoxSize));

        // Increment box position
        itemBoxPosition.x += itemBoxSize + itemBoxSpacing;

        currentRowIndex++;
        if (currentRowIndex >= ITEM_BOX_PER_ROW)
        {
            // Increment to next row
            currentRowIndex = 0;
            itemBoxPosition.x = itemBoxPadding;
            itemBoxPosition.y += itemBoxSize + itemBoxSpacing;
        }
    }

    // Start armour boxes at right end of inventory, one item box down
    pl::Vector2f armourBoxPosition(ITEM_BOX_PER_ROW * (itemBoxSize + itemBoxSpacing) + itemBoxPadding, itemBoxPadding + itemBoxSize + itemBoxSpacing);

    // Initialise armour item slots
    for (int i = 0; i < ARMOUR_SLOTS; i++)
    {
        armourItemSlots.push_back(ItemSlot(armourBoxPosition, itemBoxSize));
        
        armourBoxPosition.y += itemBoxSize + itemBoxSpacing;
    }

    // Initialise bin item slot
    pl::Vector2f binPos;
    binPos.x = (itemBoxPadding + (itemBoxSize + itemBoxSpacing) * static_cast<float>(ITEM_BOX_PER_ROW + 1));
    binPos.y = itemBoxPadding;
    binItemSlot = {ItemSlot(binPos, itemBoxSize)};
}

void InventoryGUI::initialiseHotbar()
{
    pl::Vector2f itemBoxPosition = pl::Vector2f(itemBoxPadding, itemBoxPadding);

    for (int itemIndex = 0; itemIndex < ITEM_BOX_PER_ROW; itemIndex++)
    {
        hotbarItemSlots.push_back(ItemSlot(itemBoxPosition, itemBoxSize));

        // Increment box position
        itemBoxPosition.x += itemBoxSize + itemBoxSpacing;
    }
}

void InventoryGUI::createRecipeItemSlots(InventoryData& inventory)
{
    // Calculate starting x position of recipes
    int xPos = itemBoxPadding;

    // Calculate y position
    int yPos = itemBoxPadding + (itemBoxSize + itemBoxSpacing) * std::round(inventory.getSize() / ITEM_BOX_PER_ROW) - itemBoxSpacing;
    yPos += itemBoxSize;

    recipeItemSlots.clear();

    int rowIndex = 0;

    // Create recipe item slots
    for (int i = recipeCurrentPage * RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW;
         i < std::min(static_cast<int>(availableRecipes.size()), (recipeCurrentPage + 1) * RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW); i++)
    {
        ItemSlot recipeItemSlot(pl::Vector2f(xPos, yPos), itemBoxSize);

        recipeItemSlots.push_back(recipeItemSlot);

        // Increment x position
        xPos += itemBoxSize + itemBoxSpacing;

        rowIndex++;
        if (rowIndex >= ITEM_BOX_PER_ROW)
        {
            rowIndex = 0;
            yPos += itemBoxSize + itemBoxSpacing;
            xPos = itemBoxPadding;
        }
    }

    if (controllerSelectedItemSlots == &recipeItemSlots && recipeItemSlots.size() == 0)
    {
        controllerSelectedItemSlots = &inventoryItemSlots;
        controllerSelectedSlotIndex = 0;
    }
}

void InventoryGUI::updateInventory(pl::Vector2f mouseScreenPos, float dt, InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData)
{
    // Update controller navigation
    if (!InputManager::isControllerActive())
    {
        controllerSelectedItemSlots = nullptr;
        controllerSelectedSlotIndex = 0;
    }
    else
    {
        // If controller is being used, do not update item slots based on mouse position
        mouseScreenPos = pl::Vector2f(0, 0);
    }

    // Update bin animation
    int binAnimationUpdateDirection = 0;
    if (isBinSelected(mouseScreenPos) && isItemPickedUp)
    {
        binAnimationUpdateDirection = 1;
        binScale = Helper::lerp(binScale, BIN_HOVERED_SCALE, BIN_HOVERED_SCALE_LERP_WEIGHT * dt);
    }
    else
    {
        binAnimationUpdateDirection = -1;
        binScale = Helper::lerp(binScale, 1.0f, BIN_HOVERED_SCALE_LERP_WEIGHT * dt);
    }

    binAnimation.update(dt, binAnimationUpdateDirection);

    // Update inventory item slots
    for (int i = 0; i < std::min(inventory.getSize(), static_cast<int>(inventoryItemSlots.size())); i++)
    {
        bool selectedByController = false;
        if (controllerSelectedItemSlots == &inventoryItemSlots)
        {
            selectedByController = i == controllerSelectedSlotIndex;
        }

        inventoryItemSlots[i].update(mouseScreenPos, dt, selectedByController);
        
        if (!inventory.getItemSlotData(i).has_value())
        {
            inventoryItemSlots[i].overrideItemScaleMult(1.0f);
        }
    }

    // Update armour item slots
    for (int i = 0; i < std::min(armourInventory.getSize(), static_cast<int>(armourItemSlots.size())); i++)
    {
        bool selectedByController = false;
        if (controllerSelectedItemSlots == &armourItemSlots)
        {
            selectedByController = i == controllerSelectedSlotIndex;
        }

        armourItemSlots[i].update(mouseScreenPos, dt, selectedByController);

        if (!armourInventory.getItemSlotData(i).has_value())
        {
            armourItemSlots[i].overrideItemScaleMult(1.0f);
        }
    }

    // Update bin item slot
    binItemSlot[0].update(mouseScreenPos, dt);

    // Update recipe item slots
    for (int i = 0; i < recipeItemSlots.size(); i++)
    {
        ItemSlot& itemSlot = recipeItemSlots[i];

        bool selectedByController = false;
        if (controllerSelectedItemSlots == &recipeItemSlots)
        {
            selectedByController = i == controllerSelectedSlotIndex;
        }

        // bool selected = (i == selectedRecipe);

        // itemSlot.update(mouseScreenPos, dt, selected);
        itemSlot.update(mouseScreenPos, dt, selectedByController);
    }

    // Update chest item slots
    for (int i = 0; i < chestItemSlots.size(); i++)
    {
        bool selectedByController = false;
        if (controllerSelectedItemSlots == &chestItemSlots)
        {
            selectedByController = i == controllerSelectedSlotIndex;
        }

        chestItemSlots[i].update(mouseScreenPos, dt, selectedByController);
        
        if (chestData)
        {
            int index = std::min(chestData->getSize() - 1, i);
            if (!chestData->getItemSlotData(index).has_value())
            {
                chestItemSlots[i].overrideItemScaleMult(1.0f);
            }
        }
    }

    // Notify new recipes
    recipeSeenNotifyCooldown = std::max(recipeSeenNotifyCooldown - dt, 0.0f);
    if (recipeSeenNotifyCooldown <= 0.0f && !recipesSeenToNotify.empty())
    {
        ItemType recipeItem = recipesSeenToNotify.top();
        recipesSeenToNotify.pop();

        const ItemData& itemData = ItemDataLoader::getItemData(recipeItem);
        pushItemPopup(ItemCount(recipeItem, 1), false, "NEW RECIPE - " + itemData.name);

        // Play sound
        Sounds::playSound(SoundType::Notify0, 30.0f);

        recipeSeenNotifyCooldown = MAX_RECIPE_SEEN_NOTIFY_COOLDOWN;
    }
}

void InventoryGUI::handleLeftClick(Game& game, pl::Vector2f mouseScreenPos, bool shiftMode, NetworkHandler& networkHandler,
    InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData)
{
    if (isItemPickedUp)
    {
        // If in shop, buy another of same item (if holding same item)
        int shopHoveredIndex = getHoveredItemSlotIndex(chestItemSlots, mouseScreenPos);
        if (openShopData.has_value() && shopHoveredIndex >= 0)
        {
            std::optional<ItemCount>& itemSlotData = openShopData->getItemSlotData(shopHoveredIndex);

            if (itemSlotData.has_value())
            {
                if (itemSlotData->first == pickedUpItem)
                {
                    pickUpItem(game, mouseScreenPos, 999999999, inventory, armourInventory, chestData);
                }
                else
                {
                    attemptSellItemHeld(inventory);
                }
            }
        }
        else
        {
            putDownItem(game, mouseScreenPos, inventory, armourInventory, chestData);
        }
    }
    else
    {
        // Pickup max items possible
        if (canQuickTransfer(mouseScreenPos, shiftMode, inventory, chestData))
        {
            inventoryChestItemQuickTransfer(game, mouseScreenPos, 999999999, inventory, *chestData);
        }
        else
        {
            pickUpItem(game, mouseScreenPos, 999999999, inventory, armourInventory, chestData);
        }
    }

    int recipeSlotClicked = getHoveredItemSlotIndex(recipeItemSlots, mouseScreenPos);
    if (recipeSlotClicked >= 0)
    {
        // If clicked on recipe is selected, attempt to craft item
        // if (recipeClicked == selectedRecipe)
        // {
        //     craftSelectedRecipe(inventory);
        // }
        // else
        // {
        //     // Change selected recipe to recipe clicked on
        //     selectedRecipe = recipeClicked;
        // }
        int recipeClicked = recipeSlotClicked + recipeCurrentPage * RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW;
        craftRecipe(inventory, recipeClicked);
    }

    networkHandler.queueSendPlayerData();
}

void InventoryGUI::handleRightClick(Game& game, pl::Vector2f mouseScreenPos, bool shiftMode, NetworkHandler& networkHandler,
    InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData)
{
    if (canQuickTransfer(mouseScreenPos, shiftMode, inventory, chestData) && !openShopData.has_value())
    {
        inventoryChestItemQuickTransfer(game, mouseScreenPos, 1, inventory, *chestData);
    }
    else
    {
        int shopHoveredIndex = getHoveredItemSlotIndex(chestItemSlots, mouseScreenPos);
        if (!openShopData.has_value() || shopHoveredIndex < 0)
        {
            // Only pickup single item if not attempting to buy from shop
            pickUpItem(game, mouseScreenPos, 1, inventory, armourInventory, chestData);
        }
    }

    networkHandler.queueSendPlayerData();
}

bool InventoryGUI::handleScroll(pl::Vector2f mouseScreenPos, int direction, InventoryData& inventory)
{
    // Mouse must be over recipe slots
    if (getHoveredItemSlotIndex(recipeItemSlots, mouseScreenPos) < 0)
        return false;
    
    // if (availableRecipes.size() > 0)
    // {
    //     int recipeCount = availableRecipes.size();
    //     selectedRecipe = ((selectedRecipe + direction) % recipeCount + recipeCount) % recipeCount;
    // }

    int recipePageBefore = recipeCurrentPage;

    recipeCurrentPage = std::clamp(recipeCurrentPage + direction, 0, static_cast<int>(std::floor((availableRecipes.size() - 1) / (RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW))));

    if (recipeCurrentPage != recipePageBefore)
    {
        createRecipeItemSlots(inventory);
    }

    return true;
}

void InventoryGUI::pickUpItem(Game& game, pl::Vector2f mouseScreenPos, unsigned int amount, InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData)
{
    // Get item selected at mouse
    int itemIndex = getHoveredItemSlotIndex(inventoryItemSlots, mouseScreenPos);
    int armourHoveredIndex = getHoveredItemSlotIndex(armourItemSlots, mouseScreenPos);
    int chestHoveredItemIndex = getHoveredItemSlotIndex(chestItemSlots, mouseScreenPos);

    // No valid item selected
    if (itemIndex < 0 && armourHoveredIndex < 0 && chestHoveredItemIndex < 0)
        return;
    
    bool takenFromShop = false;
    
    InventoryData* hoveredInventory = &inventory;
    if (armourHoveredIndex >= 0)
    {
        hoveredInventory = &armourInventory;
        itemIndex = armourHoveredIndex;
    }
    else if (chestHoveredItemIndex >= 0)
    {
        if (openShopData.has_value())
        {
            if (!attemptPurchaseItem(inventory, chestHoveredItemIndex))
            {
                return;
            }

            itemIndex = chestHoveredItemIndex;
            hoveredInventory = &openShopData.value();
            takenFromShop = true;
        }
        else if (chestData != nullptr)
        {
            itemIndex = chestHoveredItemIndex;
            hoveredInventory = chestData;
        }
    }
    
    std::optional<ItemCount>& itemSlotData = hoveredInventory->getItemSlotData(itemIndex);

    // If no item, do not pick up
    if (!itemSlotData.has_value())
        return;
    
    const ItemCount& itemCount = itemSlotData.value();

    const ItemData& itemData = ItemDataLoader::getItemData(itemCount.first);

    // Pick up item
    if (isItemPickedUp)
    {
        // Add to stack already in hand
        if (pickedUpItem == itemCount.first)
        {
            unsigned int amountPickedUp = std::min(amount, itemData.maxStackSize - pickedUpItemCount);

            pickedUpItemCount += std::min(itemCount.second, amountPickedUp);

            // Take from inventory / chest
            if (!takenFromShop)
            {
                hoveredInventory->takeItemAtIndex(itemIndex, amountPickedUp);
            }
        }
    }
    else
    {
        // Create new stack in hand
        isItemPickedUp = true;
        pickedUpItem = itemCount.first;
        pickedUpItemCount = std::min(itemCount.second, amount);

        // Take from inventory / chest
        // Do not take if taken (bought) from shop
        if (!takenFromShop)
        {
            hoveredInventory->takeItemAtIndex(itemIndex, amount);
        }
    }

    // Alert game of chest data modified
    if (hoveredInventory == chestData)
    {
        game.openedChestDataModified();
    }
}

void InventoryGUI::putDownItem(Game& game, pl::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData)
{
    if (!isItemPickedUp)
        return;

    // If bin selected, delete item and return
    if (isBinSelected(mouseScreenPos))
    {
        isItemPickedUp = false;
        pickedUpItem = -1;
        pickedUpItemCount = 0;

        if (InputManager::isControllerActive())
        {
            controllerSelectedItemSlots = nullptr;
            controllerSelectedSlotIndex = 0;
        }
        return;
    }

    // Get item selected at mouse
    int itemIndex = getHoveredItemSlotIndex(inventoryItemSlots, mouseScreenPos);
    int armourHoveredIndex = getHoveredItemSlotIndex(armourItemSlots, mouseScreenPos);
    int chestHoveredItemIndex = getHoveredItemSlotIndex(chestItemSlots, mouseScreenPos);

    // No valid item selected
    if (itemIndex < 0 && armourHoveredIndex < 0 && chestHoveredItemIndex < 0)
        return;

    InventoryData* hoveredInventory = &inventory;
    if (armourHoveredIndex >= 0)
    {
        // Check can put down armour
        if (!canPutDownItemInArmourInventory(armourHoveredIndex))
        {
            return;
        }

        hoveredInventory = &armourInventory;
        itemIndex = armourHoveredIndex;
    }
    else if (chestHoveredItemIndex >= 0 && chestData != nullptr)
    {
        hoveredInventory = chestData;
        itemIndex = chestHoveredItemIndex;
    }

    std::optional<ItemCount>& itemSlotData = hoveredInventory->getItemSlotData(itemIndex);

    // If item at selected position, attempt to add to stack
    if (itemSlotData.has_value())
    {
        const ItemCount& itemCount = itemSlotData.value();

        const ItemData& itemData = ItemDataLoader::getItemData(itemCount.first);

        // Item at selected position is same as in cursor, so add
        // Don't add if stack is full, swap instead
        if (pickedUpItem == itemCount.first && itemCount.second < itemData.maxStackSize)
        {
            int amountAddedToStack = std::min(itemCount.second + pickedUpItemCount, itemData.maxStackSize) - itemCount.second;

            // Add to stack
            hoveredInventory->addItemAtIndex(itemIndex, pickedUpItem, amountAddedToStack);

            // Take from cursor
            pickedUpItemCount -= amountAddedToStack;

            if (pickedUpItemCount <= 0)
                isItemPickedUp = false;
        }
        else
        {
            // Swap picked up item with selected
            // Copy item ready for swap
            ItemCount toSwap = itemCount;

            // Swap in inventory / chest
            hoveredInventory->takeItemAtIndex(itemIndex, itemCount.second);
            hoveredInventory->addItemAtIndex(itemIndex, pickedUpItem, pickedUpItemCount);

            // Swap item picked up
            pickedUpItem = toSwap.first;
            pickedUpItemCount = toSwap.second;
        }
    }
    else
    {
        const ItemData& pickedUpItemData = ItemDataLoader::getItemData(pickedUpItem);
    
        // No item at selected position, so attempt to put down whole stack
        int amountPutDown = std::min(pickedUpItemCount, static_cast<int>(pickedUpItemData.maxStackSize));
    
        // Create stack
        hoveredInventory->addItemAtIndex(itemIndex, pickedUpItem, amountPutDown);
    
        // Take from cursor
        pickedUpItemCount -= amountPutDown;
    
        if (pickedUpItemCount <= 0)
            isItemPickedUp = false;
    }

    // Alert game of chest inventory modification
    if (hoveredInventory == chestData)
    {
        game.openedChestDataModified();
    }
}

int InventoryGUI::getHoveredItemSlotIndex(const std::vector<ItemSlot>& itemSlots, pl::Vector2f mouseScreenPos)
{
    if (InputManager::isControllerActive() && controllerSelectedItemSlots == &itemSlots)
    {
        return controllerSelectedSlotIndex;
    }

    for (int itemIndex = 0; itemIndex < itemSlots.size(); itemIndex++)
    {
        const ItemSlot& itemSlot = itemSlots[itemIndex];

        if (itemSlot.isHovered())
        {
            return itemIndex;
        }
    }

    // Default case
    return -1;
}

bool InventoryGUI::isBinSelected(pl::Vector2f mouseScreenPos)
{
    return (getHoveredItemSlotIndex(binItemSlot, mouseScreenPos) >= 0);
}

bool InventoryGUI::isInventorySelected(pl::Vector2f mouseScreenPos)
{
    return (getHoveredItemSlotIndex(inventoryItemSlots, mouseScreenPos) >= 0);
}

bool InventoryGUI::isCraftingSelected(pl::Vector2f mouseScreenPos)
{
    return (getHoveredItemSlotIndex(recipeItemSlots, mouseScreenPos) >= 0);
}

void InventoryGUI::craftRecipe(InventoryData& inventory, int selectedRecipe)
{
    // Get recipe data
    int recipeIdx = availableRecipes[selectedRecipe];
    const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipeIdx];

    // If item in hand and is not item to be crafted, do not craft recipe (as won't be able to pick up crafted item)
    if (isItemPickedUp)
    {
        const ItemData& pickedUpItemData = ItemDataLoader::getItemData(pickedUpItem);

        if (pickedUpItem != recipeData.product || pickedUpItemCount > pickedUpItemData.maxStackSize - recipeData.productAmount)
        {
            return;
        }
    }

    // Get inventory to check if has items (should have items as recipe is in available recipes, check just in case)
    std::unordered_map<ItemType, unsigned int> inventoryItemCount = inventory.getTotalItemCount();

    // Check has items
    for (const auto& itemRequired : recipeData.itemRequirements)
    {
        if (inventoryItemCount.count(itemRequired.first) <= 0)  
            return;
        
        if (inventoryItemCount[itemRequired.first] < itemRequired.second)
            return;
    }

    // Take items
    for (const auto& itemRequired : recipeData.itemRequirements)
    {
        inventory.takeItem(itemRequired.first, itemRequired.second);
    }

    // Give crafted item to player
    if (isItemPickedUp)
    {
        pickedUpItemCount += recipeData.productAmount;
    }
    else
    {
        isItemPickedUp = true;
        pickedUpItem = recipeData.product;
        pickedUpItemCount = recipeData.productAmount;
    }

    // Play craft sound
    int soundChance = rand() % 2;
    SoundType craftSound = SoundType::CraftBuild1;
    if (soundChance == 1) craftSound = SoundType::CraftBuild2;

    Sounds::playSound(craftSound, 60.0f);

    // Inventory changed, so update available recipes
    // Use previous crafting station levels stored as only updating for item change
    updateAvailableRecipes(inventory, previous_nearbyCraftingStationLevels);
}

void InventoryGUI::handleClose(InventoryData& inventory, InventoryData* chestData)
{
    // Handle item still picked up when inventory is closed
    if (isItemPickedUp)
    {
        isItemPickedUp = false;

        inventory.addItem(pickedUpItem, pickedUpItemCount);
    }
}

bool InventoryGUI::isMouseOverUI(pl::Vector2f mouseScreenPos)
{
    return (isInventorySelected(mouseScreenPos) || isCraftingSelected(mouseScreenPos) || isBinSelected(mouseScreenPos) || (getHoveredItemSlotIndex(armourItemSlots, mouseScreenPos) >= 0) ||
        (getHoveredItemSlotIndex(chestItemSlots, mouseScreenPos) >= 0));
}

void InventoryGUI::updateAvailableRecipes(InventoryData& inventory, std::unordered_map<std::string, int> nearbyCraftingStationLevels)
{
    std::unordered_map<ItemType, unsigned int> inventoryItemCount = inventory.getTotalItemCount();

    // If crafting stations and items have not changed, do not update recipes
    if (nearbyCraftingStationLevels == previous_nearbyCraftingStationLevels && inventoryItemCount == previous_inventoryItemCount)
        return;
    
    previous_nearbyCraftingStationLevels = nearbyCraftingStationLevels;
    previous_inventoryItemCount = inventoryItemCount;

    std::vector<int> previous_availableRecipes = availableRecipes;

    // Reset available recipes
    availableRecipes.clear();

    std::vector<int> partiallyAvailableRecipes;

    // Iterate over all recipes and determine if available to player
    for (int recipeIdx = 0; recipeIdx < RecipeDataLoader::getRecipeData().size(); recipeIdx++)
    {
        // Get recipe data
        const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipeIdx];

        // If crafting station required not nearby, do not add to recipes
        if (!recipeData.craftingStationRequired.empty())
        {
            if (nearbyCraftingStationLevels.count(recipeData.craftingStationRequired) <= 0)
                continue;

            if (nearbyCraftingStationLevels[recipeData.craftingStationRequired] < recipeData.craftingStationLevelRequired)
                continue; 
        }

        // Check items - add recipe if player has at least one of required items
        bool hasItems = true;
        bool hasItemType = false;
        for (const auto& itemRequired : recipeData.itemRequirements)
        {
            if (inventoryItemCount.count(itemRequired.first) > 0)
            {
                hasItemType = true;
            }
            // If player does not have any of the item, cannot craft
            if (inventoryItemCount.count(itemRequired.first) <= 0)
            {
                hasItems = false;
            }
            else
            {
                // If player has item but not enough, cannot craft
                if (inventoryItemCount[itemRequired.first] < itemRequired.second)
                {
                    hasItems = false;
                }
            }
        }

        // Check has key items (must have all)
        if (recipeData.keyItems.has_value())
        {
            hasItemType = true;
            for (const auto& keyItem : recipeData.keyItems.value())
            {
                if (inventoryItemCount.count(keyItem) <= 0)
                {
                    hasItemType = false;
                    break;
                }
            }
        }

        if (!hasItems)
        {
            if (hasItemType)
            {
                partiallyAvailableRecipes.push_back(recipeIdx);   
            }
            continue;
        }
        
        // Player has items and is near required crafting station, so add to available recipes
        availableRecipes.push_back(recipeIdx);
    }

    availableRecipes.insert(availableRecipes.end(), partiallyAvailableRecipes.begin(), partiallyAvailableRecipes.end());

    // Update UI if required
    if (availableRecipes != previous_availableRecipes)
    {
        // if (availableRecipes.size() > 0)
        // {
        //     selectedRecipe = (selectedRecipe % availableRecipes.size() + availableRecipes.size()) % availableRecipes.size();
        // }
        // else
        // {
        //     selectedRecipe = 0;
        // }
        recipeCurrentPage = std::clamp(recipeCurrentPage, 0, static_cast<int>(std::floor(
            std::max(static_cast<int>(availableRecipes.size() - 1) / (RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW), 0))));

        createRecipeItemSlots(inventory);

        // Add any new recipes to "recipes seen" set and create popup
        for (int recipe : availableRecipes)
        {
            const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipe];
            if (recipesSeen.contains(recipeData.product))
            {
                continue;
            }

            recipesSeenToNotify.push(recipeData.product);
            recipesSeen.insert(recipeData.product);
        }
    }
}

void InventoryGUI::reset()
{
    recipesSeen = std::unordered_set<ItemType>();
    recipesSeenToNotify = std::stack<ItemType>();
    recipeSeenNotifyCooldown = 0.0f;
    itemPopups.clear();
    selectedHotbarIndex = 0;
}

void InventoryGUI::setSeenRecipes(const std::unordered_set<ItemType>& recipes)
{
    recipesSeen = recipes;
}

const std::unordered_set<ItemType>& InventoryGUI::getSeenRecipes()
{
    return recipesSeen;
}

ItemType InventoryGUI::getHeldItemType(InventoryData& inventory)
{
    if (isItemPickedUp)
    {
        return pickedUpItem;
    }

    std::optional<ItemCount>& selectedItemSlot = inventory.getItemSlotData(selectedHotbarIndex);

    if (selectedItemSlot.has_value())
    {
        return selectedItemSlot->first;
    }

    // Default case
    return -1;
}

ObjectType InventoryGUI::getHeldObjectType(InventoryData& inventory, bool isInInventory)
{
    if (isItemPickedUp)
    {
        ObjectType placeObjectType = ItemDataLoader::getItemData(pickedUpItem).placesObjectType;
        return placeObjectType;
    }

    // Get from hotbar as no item is picked up
    if (!isInInventory)
    {
        return getHotbarSelectedObject(inventory);
    }

    return -1;
}

ToolType InventoryGUI::getHeldToolType(InventoryData& inventory)
{
    if (isItemPickedUp)
    {
        ToolType toolType = ItemDataLoader::getItemData(pickedUpItem).toolType;
        return toolType;
    }
    
    // Get from hotbar as no item is picked up
    return getHotbarSelectedTool(inventory);
}

void InventoryGUI::subtractHeldItem(InventoryData& inventory)
{
    if (isItemPickedUp)
    {
        pickedUpItemCount--;

        if (pickedUpItemCount <= 0)
        {
            isItemPickedUp = false;
            pickedUpItem = -1;
            pickedUpItemCount = 0;
        }
        return;
    }
    
    // Take from hotbar
    subtractHotbarItem(inventory);
}

bool InventoryGUI::heldItemPlacesLand(InventoryData& inventory, bool isInInventory)
{
    if (isItemPickedUp)
    {
        const ItemData& itemData = ItemDataLoader::getItemData(pickedUpItem);
        return itemData.placesLand;
    }

    // Get from hotbar
    if (!isInInventory)
    {
        return hotbarItemPlacesLand(inventory);
    }

    return false;
}

void InventoryGUI::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, float gameTime, pl::Vector2f mouseScreenPos,
    InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData)
{
    if (openShopData.has_value())
    {
        assert(chestData == nullptr);
    }

    drawInventory(window, spriteBatch, inventory);
    drawArmourInventory(window, spriteBatch, armourInventory);
    drawBin(window, spriteBatch);

    if (openShopData.has_value())
    {
        drawChest(window, spriteBatch, &openShopData.value());
    }
    else
    {
        drawChest(window, spriteBatch, chestData);
    }

    drawRecipes(window, spriteBatch);

    drawPickedUpItem(window, spriteBatch, gameTime, mouseScreenPos);

    spriteBatch.endDrawing(window);

    drawHoveredItemInfoBox(window, gameTime, mouseScreenPos, inventory, armourInventory, chestData);
}

void InventoryGUI::drawInventory(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, InventoryData& inventory)
{
    for (int itemIdx = 0; itemIdx < inventoryItemSlots.size(); itemIdx++)
    {
        const std::optional<ItemCount>& itemSlotData = inventory.getItemSlotData(itemIdx);

        ItemSlot& itemSlot = inventoryItemSlots[itemIdx];

        bool selectedByController = false;
        if (controllerSelectedItemSlots == &inventoryItemSlots)
        {
            selectedByController = itemIdx == controllerSelectedSlotIndex;
        }

        if (itemSlotData.has_value())
        {
            const ItemCount& itemCount = itemSlotData.value();

            itemSlot.draw(window, spriteBatch, itemCount.first, itemCount.second, false, selectedByController, std::nullopt, &inventory);
        }
        else
        {
            // Draw blank item box
            itemSlot.draw(window, spriteBatch, std::nullopt, std::nullopt, false, selectedByController);
        }
    }
}

void InventoryGUI::drawArmourInventory(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, InventoryData& armourInventory)
{
    static const std::array<pl::Rect<int>, ARMOUR_SLOTS> emptyArmourSlotIcons = {{
        {176, 32, 15, 10},
        {192, 32, 11, 10},
        {208, 32, 9, 9}
    }};

    static const pl::Rect<int> defenceIconTextureRect = pl::Rect<int>(160, 32, 16, 16);

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Draw defence
    pl::Vector2f defenceDisplayPos;
    defenceDisplayPos.x = (itemBoxPadding + (itemBoxSize + itemBoxSpacing) * (static_cast<float>(ITEM_BOX_PER_ROW) + 0.5f)) * intScale;
    defenceDisplayPos.y = (itemBoxPadding + itemBoxSize * 0.5f) * intScale;

    pl::DrawData defenceDrawData;
    defenceDrawData.texture = TextureManager::getTexture(TextureType::UI);
    defenceDrawData.shader = Shaders::getShader(ShaderType::Default);
    defenceDrawData.position = defenceDisplayPos;
    defenceDrawData.scale = pl::Vector2f(3, 3) * intScale;
    defenceDrawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    defenceDrawData.textureRect = defenceIconTextureRect;

    TextureManager::drawSubTexture(window, defenceDrawData);

    // Draw defence text
    int defence = PlayerStats::calculateDefence(armourInventory);

    pl::TextDrawData defenceTextDrawData;
    defenceTextDrawData.position = defenceDisplayPos + pl::Vector2f(itemBoxSize, itemBoxSize) * 0.25f * intScale;
    defenceTextDrawData.size = 24 * static_cast<unsigned int>(intScale);
    defenceTextDrawData.color = pl::Color(255, 255, 255);
    defenceTextDrawData.outlineColor = pl::Color(46, 34, 47);
    defenceTextDrawData.outlineThickness = 2 * intScale;
    defenceTextDrawData.text = std::to_string(defence);
    defenceTextDrawData.centeredX = true;
    defenceTextDrawData.centeredY = true;

    TextDraw::drawText(window, defenceTextDrawData);

    // Draw armour slots
    for (int i = 0; i < std::min(armourInventory.getSize(), static_cast<int>(armourItemSlots.size())); i++)
    {
        const std::optional<ItemCount>& itemSlotData = armourInventory.getItemSlotData(i);

        ItemSlot& itemSlot = armourItemSlots[i];

        bool selectedByController = false;
        if (controllerSelectedItemSlots == &armourItemSlots)
        {
            selectedByController = i == controllerSelectedSlotIndex;
        }

        if (itemSlotData.has_value())
        {
            const ItemCount& itemCount = itemSlotData.value();

            itemSlot.draw(window, spriteBatch, itemCount.first, itemCount.second, false, selectedByController);
        }
        else
        {
            itemSlot.draw(window, spriteBatch, std::nullopt, std::nullopt, false, selectedByController, emptyArmourSlotIcons[i]);
        }
    }
}

void InventoryGUI::drawBin(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    pl::Vector2f binPosition;
    binPosition.x = binItemSlot[0].getPosition().x * intScale + binItemSlot[0].getItemBoxSize() / 2;
    binPosition.y = binItemSlot[0].getPosition().y * intScale + binItemSlot[0].getItemBoxSize() / 2;

    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::UI);
    drawData.shader = Shaders::getShader(ShaderType::Default);
    drawData.position = binPosition;
    drawData.scale = pl::Vector2f(3, 3) * binScale * intScale;
    drawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    drawData.textureRect = binAnimation.getTextureRect();

    spriteBatch.draw(window, drawData);
}

void InventoryGUI::drawRecipes(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    if (recipeItemSlots.size() > 0)
    {
        // Store recipe index hovered over for drawing
        // int hoveredRecipeIdx = -1;

        // Draw recipes
        for (int i = recipeCurrentPage * RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW;
             i < std::min(static_cast<int>(availableRecipes.size()), (recipeCurrentPage + 1) * RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW); i++)
        {
            // Get recipe index
            int recipeIdx = availableRecipes[i];

            // Get recipe data
            const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipeIdx];

            int recipeSlotIdx = i % (RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW);

            // Get item slot
            ItemSlot& itemSlot = recipeItemSlots[recipeSlotIdx];

            bool selectedByController = false;
            if (controllerSelectedItemSlots == &recipeItemSlots)
            {
                selectedByController = recipeSlotIdx == controllerSelectedSlotIndex;
            }

            // If recipe is selected, draw requirements
            // bool selected = i == selectedRecipe;

            // Draw item box for product
            // itemSlot.draw(window, recipeData.product, recipeData.productAmount, false, selected);
            itemSlot.draw(window, spriteBatch, recipeData.product, recipeData.productAmount, false, selectedByController);
            
            // Test whether mouse is over - if so, store recipe index for drawing info later
            // if (itemSlot.isHovered())
            // {
            //     hoveredRecipeIdx = recipeIdx;
            // }
        }

        // Draw hammer icon
        pl::DrawData hammerDrawData;
        hammerDrawData.texture = TextureManager::getTexture(TextureType::UI);
        hammerDrawData.shader = Shaders::getShader(ShaderType::Default);
        hammerDrawData.position = (recipeItemSlots.back().getPosition() + pl::Vector2f(itemBoxSize + itemBoxSpacing, 0)) * intScale;
        hammerDrawData.scale = pl::Vector2f(3, 3) * intScale;
        hammerDrawData.textureRect = pl::Rect<int>(80, 16, 16, 16);

        spriteBatch.draw(window, hammerDrawData);

        static constexpr int RECIPE_PAGE_COUNT_HAMMMER_OFFSET_Y = 45;

        int recipePageCount = static_cast<int>(std::floor((availableRecipes.size() - 1) / (RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW))) + 1;
        if (recipePageCount > 1)
        {
            // Draw recipe page index
            pl::TextDrawData textDrawData;
            textDrawData.text = std::to_string(recipeCurrentPage + 1) + "/" + std::to_string(recipePageCount);
            textDrawData.position = (recipeItemSlots.back().getPosition() + pl::Vector2f(itemBoxSize + itemBoxSpacing + 10, RECIPE_PAGE_COUNT_HAMMMER_OFFSET_Y)) * intScale;
            textDrawData.color = pl::Color(255, 255, 255);
            textDrawData.size = 24 * intScale;
            textDrawData.outlineColor = pl::Color(46, 34, 47);
            textDrawData.outlineThickness = 2 * intScale;
            TextDraw::drawText(window, textDrawData);
        }

        // Draw info of recipe hovered over (if any)
        // if (hoveredRecipeIdx >= 0 && !isItemPickedUp)
        // {
        //     drawItemInfoBoxRecipe(window, gameTime, hoveredRecipeIdx, mouseScreenPos);
        // }
    }
}

void InventoryGUI::drawChest(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, InventoryData* chestData)
{
    if (chestData == nullptr)
    {
        return;
    }

    for (int itemIdx = 0; itemIdx < chestItemSlots.size(); itemIdx++)
    {
        const std::optional<ItemCount>& itemSlotData = chestData->getItemSlotData(itemIdx);

        ItemSlot& itemSlot = chestItemSlots[itemIdx];

        bool selectedByController = false;
        if (controllerSelectedItemSlots == &chestItemSlots)
        {
            selectedByController = itemIdx == controllerSelectedSlotIndex;
        }

        if (itemSlotData.has_value())
        {
            const ItemCount& itemCount = itemSlotData.value();

            itemSlot.draw(window, spriteBatch, itemCount.first, itemCount.second, false, selectedByController);
        }
        else
        {
            // Draw blank item box
            itemSlot.draw(window, spriteBatch, std::nullopt, std::nullopt, false, selectedByController);
        }
    }
}

void InventoryGUI::drawPickedUpItem(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, float gameTime, pl::Vector2f mouseScreenPos)
{
    if (!isItemPickedUp)
    {
        return;
    }

    // Override mouse screen pos if using controller
    if (InputManager::isControllerActive() && controllerSelectedItemSlots != nullptr)
    {
        float intScale = ResolutionHandler::getResolutionIntegerScale();
        ItemSlot& itemSlot = controllerSelectedItemSlots->at(controllerSelectedSlotIndex);
        int itemBoxOffset = itemSlot.getItemBoxSize() * 0.75f;
        mouseScreenPos = (itemSlot.getPosition() + pl::Vector2f(itemBoxOffset, itemBoxOffset)) * intScale;
    }

    // Create dummy item slot at cursor
    ItemSlot pickedUpItemSlot(mouseScreenPos, itemBoxSize, false);
    pickedUpItemSlot.overrideItemScaleMult(1.0f + std::pow(std::sin(gameTime * 3.5f), 2) / 8.0f);
    pickedUpItemSlot.draw(window, spriteBatch, pickedUpItem, pickedUpItemCount, true);
}

void InventoryGUI::drawHoveredItemInfoBox(pl::RenderTarget& window, float gameTime, pl::Vector2f mouseScreenPos, InventoryData& inventory,
    InventoryData& armourInventory, InventoryData* chestData)
{
    // Do not draw info box if an item is picked up
    if (isItemPickedUp)
    {
        return;
    }

    // Create texture to draw info box on
    // Allows info box to be kept on screen
    pl::Framebuffer infoBoxTexture;
    infoBoxTexture.create(window.getWidth(), window.getHeight());
    infoBoxTexture.clear(pl::Color(0, 0, 0, 0));

    pl::Vector2f infoBoxSize;

    // Get currently hovered over item
    int hoveredItemIndex = getHoveredItemSlotIndex(inventoryItemSlots, mouseScreenPos);

    // Get currently hovered armour piece
    int hoveredArmourIndex = getHoveredItemSlotIndex(armourItemSlots, mouseScreenPos);

    // Get currently hovered recipe
    int hoveredRecipeSlotIndex = getHoveredItemSlotIndex(recipeItemSlots, mouseScreenPos);

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // If using controller, override mouse position to centre of selected box
    if (InputManager::isControllerActive() && controllerSelectedItemSlots != nullptr)
    {
        ItemSlot& itemSlot = controllerSelectedItemSlots->at(controllerSelectedSlotIndex);
        int itemBoxOffset = itemSlot.getItemBoxSize() * 0.75f;
        mouseScreenPos = (itemSlot.getPosition() + pl::Vector2f(itemBoxOffset, itemBoxOffset)) * intScale;
    }

    // If an item is hovered over, draw item info box
    if (hoveredItemIndex >= 0)
    {
        infoBoxSize = drawItemInfoBox(infoBoxTexture, gameTime, hoveredItemIndex, inventory, pl::Vector2f(0, 0), InventoryShopInfoMode::Sell);
    }
    else if (hoveredArmourIndex >= 0)
    {
        infoBoxSize = drawItemInfoBox(infoBoxTexture, gameTime, hoveredArmourIndex, armourInventory, pl::Vector2f(0, 0), InventoryShopInfoMode::Sell);
    }
    else if (hoveredRecipeSlotIndex >= 0)
    {
        int hoveredRecipeIndex = hoveredRecipeSlotIndex + recipeCurrentPage * RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW;
        infoBoxSize = drawItemInfoBoxRecipe(infoBoxTexture, gameTime, availableRecipes[hoveredRecipeIndex], pl::Vector2f(0, 0));
    }
    else if (chestData != nullptr || openShopData.has_value())
    {
        // Draw chest hovered item info
        int chestHoveredItemIndex = getHoveredItemSlotIndex(chestItemSlots, mouseScreenPos);
        if (chestHoveredItemIndex >= 0)
        {
            if (chestData != nullptr)
            {
                infoBoxSize = drawItemInfoBox(infoBoxTexture, gameTime, chestHoveredItemIndex, *chestData, pl::Vector2f(0, 0), InventoryShopInfoMode::None);
            }
            else
            {
                infoBoxSize = drawItemInfoBox(infoBoxTexture, gameTime, chestHoveredItemIndex, openShopData.value(), pl::Vector2f(0, 0), InventoryShopInfoMode::Buy);
            }
        }
    }

    if (infoBoxSize.x > 0 && infoBoxSize.y > 0)
    {
        // Keep on screen
        pl::Vector2f drawPos = mouseScreenPos;
        drawPos.x = std::min(window.getWidth() - infoBoxSize.x, drawPos.x);
        drawPos.y = std::min(window.getHeight() - infoBoxSize.y, drawPos.y);

        pl::VertexArray box;

        box.addQuad(pl::Rect<float>(drawPos, pl::Vector2f(infoBoxTexture.getWidth(), infoBoxTexture.getHeight())), pl::Color(),
            pl::Rect<float>(0, infoBoxTexture.getHeight(), infoBoxTexture.getWidth(), -infoBoxTexture.getHeight()));

        window.draw(box, *Shaders::getShader(ShaderType::Default), &infoBoxTexture.getTexture(), pl::BlendMode::Alpha);
    }
}

pl::Vector2f InventoryGUI::drawItemInfoBox(pl::RenderTarget& window, float gameTime, int itemIndex, InventoryData& inventory, pl::Vector2f mouseScreenPos,
    InventoryShopInfoMode shopInfoMode)
{
    const std::optional<ItemCount>& itemSlot = inventory.getItemSlotData(itemIndex);

    // If no item in slot, do not draw box
    if (!itemSlot.has_value())
        return pl::Vector2f(0, 0);
    
    return drawItemInfoBox(window, gameTime, itemSlot.value(), mouseScreenPos, shopInfoMode);
}

pl::Vector2f InventoryGUI::drawItemInfoBox(pl::RenderTarget& window, float gameTime, ItemCount itemCount, pl::Vector2f mouseScreenPos, InventoryShopInfoMode shopInfoMode)
{
    ItemType itemType = itemCount.first;
    unsigned int itemAmount = itemCount.second;

    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    std::vector<ItemInfoString> infoStrings;

    pl::Color itemNameColor = itemData.getNameColor(gameTime);

    infoStrings.push_back({itemData.getDisplayName(), 24, itemNameColor});

    if (itemData.armourType >= 0)
    {
        const ArmourData& armourData = ArmourDataLoader::getArmourData(itemData.armourType);

        infoStrings.push_back({"Equippable", 20});
        infoStrings.push_back({std::to_string(armourData.defence) + " defence", 20});
    }
    else if (itemData.placesObjectType >= 0)
    {
        infoStrings.push_back({"Can be placed", 20});

        const ObjectData& objectData = ObjectDataLoader::getObjectData(itemData.placesObjectType);

        if (objectData.chestCapacity > 0)
        {
            infoStrings.push_back({std::to_string(objectData.chestCapacity) + " capacity", 20});
        }
        else if (objectData.rocketObjectData.has_value())
        {
            infoStrings.push_back({"Allows travel to:", 20});

            for (PlanetType planetDestination : objectData.rocketObjectData->availableDestinations)
            {
                const std::string& planetName = PlanetGenDataLoader::getPlanetGenData(planetDestination).displayName;
                infoStrings.push_back({" - " + planetName, 20});
            }

            for (RoomType roomDestination : objectData.rocketObjectData->availableRoomDestinations)
            {
                const std::string& roomDestinationName = StructureDataLoader::getRoomData(roomDestination).displayName;
                infoStrings.push_back({" - " + roomDestinationName, 20});
            }
        }
        else if (objectData.npcObjectData.has_value())
        {
            infoStrings.push_back({"How have you obtained this..?", 20});
        }
    }
    else if (itemData.placesLand)
    {
        infoStrings.push_back({"Can be placed", 20});
    }
    else if (itemData.toolType >= 0)
    {
        const ToolData& toolData = ToolDataLoader::getToolData(itemData.toolType);

        switch (toolData.toolBehaviourType)
        {
            case ToolBehaviourType::Pickaxe:
            {
                infoStrings.push_back({std::to_string(toolData.damage) + " damage", 20});
                break;
            }
            case ToolBehaviourType::FishingRod:
            {
                infoStrings.push_back({Helper::floatToString(toolData.fishingEfficiency, 2) + " fishing efficiency", 20});
                break;
            }
            case ToolBehaviourType::BowWeapon:
            {
                infoStrings.push_back({Helper::floatToString(toolData.projectileDamageMult, 2) + "x projectile damage", 20});
                infoStrings.push_back({Helper::floatToString(toolData.shootPower, 2) + " shooting power", 20});
                break;
            }
            case ToolBehaviourType::MeleeWeapon:
            {
                infoStrings.push_back({std::to_string(toolData.damage) + " melee damage", 20});
                break;
            }
        }
    }
    else if (itemData.projectileType >= 0)
    {
        const ProjectileData& projectileData = ToolDataLoader::getProjectileData(itemData.projectileType);

        int averageDamage = std::round((projectileData.damageHigh + projectileData.damageLow) / 2.0f);

        infoStrings.push_back({std::to_string(averageDamage) + " damage", 20});
        infoStrings.push_back({"Ammo", 20});
    }
    else if (itemData.bossSummonData.has_value())
    {
        infoStrings.push_back({"Summons " + itemData.bossSummonData->bossName, 20});
    }
    else if (itemData.consumableData.has_value())
    {
        infoStrings.push_back({"Consumable", 20});
        if (itemData.consumableData->healthIncrease > 0)
        {
            infoStrings.push_back({"+" + std::to_string(itemData.consumableData->healthIncrease) + " health", 20});
        }
        if (itemData.consumableData->permanentHealthIncrease > 0)
        {
            infoStrings.push_back({"+" + std::to_string(itemData.consumableData->permanentHealthIncrease) + " permanent max health", 20});
        }
    }

    if (itemData.currencyValue > 0)
    {
        infoStrings.push_back({std::to_string(itemData.currencyValue * itemAmount) + " currency value", 20});
    }

    if (itemData.isMaterial)
    {
        infoStrings.push_back({"Material", 20});
    }

    if (!itemData.description.empty())
    {
        infoStrings.push_back({itemData.description, 20});
    }

    if (openShopData.has_value() && shopInfoMode != InventoryShopInfoMode::None)
    {
        std::string buyInfoString;
        int itemPrice = 0;

        switch(shopInfoMode)
        {
            case InventoryShopInfoMode::Buy:
            {
                itemPrice = std::floor(openShopData->getItemBuyPrice(itemType) * itemAmount);
                buyInfoString = "Buy for " + std::to_string(itemPrice) + " currency";
                break;
            }
            case InventoryShopInfoMode::Sell:
            {
                itemPrice = std::floor(openShopData->getItemSellPrice(itemType) * itemAmount);
                buyInfoString = "Sell for " + std::to_string(itemPrice) + " currency";
                break;
            }
        }

        if (itemPrice > 0)
        {
            infoStrings.push_back({buyInfoString, 20});
        }
    }

    return drawInfoBox(window, mouseScreenPos + pl::Vector2f(8, 8) * 3.0f * intScale, infoStrings);
}

pl::Vector2f InventoryGUI::drawItemInfoBoxRecipe(pl::RenderTarget& window, float gameTime, int recipeIdx, pl::Vector2f mouseScreenPos)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipeIdx];

    pl::Vector2f itemInfoBoxSize = drawItemInfoBox(window, gameTime, {recipeData.product, recipeData.productAmount}, mouseScreenPos, InventoryShopInfoMode::None);

    std::vector<ItemInfoString> infoStrings;

    infoStrings.push_back({"Requires", 20});

    for (const auto& item : recipeData.itemRequirements)
    {
        ItemInfoString itemRequirement;
        itemRequirement.itemCount = ItemCount(item.first, item.second);

        const ItemData& itemData = ItemDataLoader::getItemData(item.first);
        itemRequirement.string = itemData.getDisplayName();
        itemRequirement.size = 20;

        itemRequirement.color = pl::Color(232, 59, 59);
        if (previous_inventoryItemCount.count(item.first) >= 0)
        {
            if (previous_inventoryItemCount[item.first] >= item.second)
            {
                itemRequirement.color = pl::Color(255, 255, 255);
            }
        }

        infoStrings.push_back(itemRequirement);
    }

    if (!recipeData.craftingStationRequired.empty())
    {
        infoStrings.push_back({"", 20});
        infoStrings.push_back({"Crafted in", 20});

        for (ItemType craftingStationItem : ItemDataLoader::getCraftingStationLevelItems(recipeData.craftingStationRequired, recipeData.craftingStationLevelRequired))
        {
            ItemInfoString craftingStationRequirement;
            craftingStationRequirement.itemCount = ItemCount(craftingStationItem, 1);

            const ItemData& itemData = ItemDataLoader::getItemData(craftingStationItem);
            craftingStationRequirement.string = itemData.getDisplayName();
            craftingStationRequirement.size = 20;

            craftingStationRequirement.drawItemCountNumberWhenOne = false;

            infoStrings.push_back(craftingStationRequirement);
        }
    }

    pl::Vector2f recipeInfoBoxSize = drawInfoBox(window, mouseScreenPos + pl::Vector2f(8, 8 + 6) * 3.0f * intScale + pl::Vector2f(0, itemInfoBoxSize.y), infoStrings);

    return (itemInfoBoxSize + recipeInfoBoxSize + pl::Vector2f(0, 8 + 6 + 5) * 3.0f * intScale);
}

pl::Vector2f InventoryGUI::drawInfoBox(pl::RenderTarget& window, pl::Vector2f position, const std::vector<ItemInfoString>& infoStrings, int alpha, float flashAmount)
{
    static const std::array<pl::Rect<int>, 4> sides = {
        pl::Rect<int>(20, 80, 1, 3), // top
        pl::Rect<int>(22, 84, 3, 1), // right
        pl::Rect<int>(20, 86, 1, 3), // bottom
        pl::Rect<int>(16, 84, 3, 1)  // left
    };
    static const std::array<pl::Rect<int>, 4> corners = {
        pl::Rect<int>(16, 80, 3, 3), // top left
        pl::Rect<int>(22, 80, 3, 3), // top right
        pl::Rect<int>(22, 86, 3, 3),  // bottom right
        pl::Rect<int>(16, 86, 3, 3) // bottom left
    };
    static const pl::Rect<int> centre(20, 84, 1, 1);

    int intScale = ResolutionHandler::getResolutionIntegerScale();

    // Get size of box
    int width = 0;
    int height = 0;
    static constexpr int textXPadding = 5;
    static constexpr int textYPadding = 10;

    static constexpr int itemSize = 16 * 3;

    for (const ItemInfoString& infoString : infoStrings)
    {
        pl::TextDrawData textDrawData = {
            .text = infoString.string,
            .size = infoString.size * static_cast<unsigned int>(intScale)
        };

        if (infoString.itemCount.has_value())
        {
            width = std::max(width, static_cast<int>(TextDraw::getTextSize(textDrawData).width) + (textXPadding * 4 + itemSize) * intScale);
            height += itemSize * intScale + textYPadding * intScale;
        }
        else
        {
            width = std::max(width, static_cast<int>(TextDraw::getTextSize(textDrawData).width) + textXPadding * 2 * intScale);
            height += TextDraw::getTextSize(textDrawData).height + textYPadding * intScale;
        }
    }

    // Draw box
    pl::SpriteBatch spriteBatch;
    
    pl::DrawData boxDrawData;
    boxDrawData.texture = TextureManager::getTexture(TextureType::UI);
    boxDrawData.shader = Shaders::getShader(ShaderType::Default);
    boxDrawData.color = pl::Color(255, 255, 255, alpha);
    
    if (flashAmount > 0.0f)
    {
        boxDrawData.shader = Shaders::getShader(ShaderType::Flash);
        boxDrawData.shader->setUniform1f("flash_amount", flashAmount);
    }
    
    pl::Vector2f scale(3 * intScale, 3 * intScale);
    boxDrawData.scale = scale;

    // Draw corners
    boxDrawData.position = position;
    boxDrawData.textureRect = corners[0];
    spriteBatch.draw(window, boxDrawData);

    boxDrawData.position = position + pl::Vector2f(width + sides[1].width * scale.y, 0);
    boxDrawData.textureRect = corners[1];
    spriteBatch.draw(window, boxDrawData);

    boxDrawData.position = position + pl::Vector2f(width + sides[1].width * scale.x, height + sides[0].height * scale.y);
    boxDrawData.textureRect = corners[2];
    spriteBatch.draw(window, boxDrawData);

    boxDrawData.position = position + pl::Vector2f(0, height + sides[0].height * scale.y);
    boxDrawData.textureRect = corners[3];
    spriteBatch.draw(window, boxDrawData);

    // Draw sides
    boxDrawData.position = position + pl::Vector2f(sides[3].width * scale.x, 0);
    boxDrawData.scale = pl::Vector2f(width, scale.y);
    boxDrawData.textureRect = sides[0];
    spriteBatch.draw(window, boxDrawData);
    
    boxDrawData.position = position + pl::Vector2f(sides[3].width * scale.x + width, sides[0].height * scale.y);
    boxDrawData.scale = pl::Vector2f(scale.x, height);
    boxDrawData.textureRect = sides[1];
    spriteBatch.draw(window, boxDrawData);
    
    boxDrawData.position = position + pl::Vector2f(sides[3].width * scale.x, sides[0].height * scale.y + height);
    boxDrawData.scale = pl::Vector2f(width, scale.y);
    boxDrawData.textureRect = sides[2];
    spriteBatch.draw(window, boxDrawData);
        
    boxDrawData.position = position + pl::Vector2f(0, sides[0].height * scale.x);
    boxDrawData.scale = pl::Vector2f(scale.x, height);
    boxDrawData.textureRect = sides[3];
    spriteBatch.draw(window, boxDrawData);

    // Draw centre
    boxDrawData.position = position + pl::Vector2f(sides[3].width * scale.x, sides[0].height * scale.y);
    boxDrawData.scale = pl::Vector2f(width, height);
    boxDrawData.textureRect = centre;
    spriteBatch.draw(window, boxDrawData);

    spriteBatch.endDrawing(window);

    // Draw text
    int textYOffset = 0;
    static constexpr int textYShift = 6;

    for (int i = 0; i < infoStrings.size(); i++)
    {
        const ItemInfoString& infoString = infoStrings[i];

        pl::TextDrawData textDrawData = {
            .text = infoString.string,
            .position = position + pl::Vector2f(sides[3].width * 3 * intScale + textXPadding * intScale,
                sides[0].height * 3 * intScale + textYOffset - textYShift * intScale),
            .color = pl::Color(infoString.color.r, infoString.color.g, infoString.color.b, alpha),
            .size = infoString.size * static_cast<unsigned int>(intScale)
        };

        if (infoString.itemCount.has_value())
        {
            const ItemCount& itemCount = infoString.itemCount.value();

            // Draw item using item slot
            // ItemSlot itemSlot(textDrawData.position, itemSize * intScale, false);
            // itemSlot.draw(window, itemCount.first, itemCount.second, true);
            // Offset text draw data position downwards
            textDrawData.position.y += textYPadding * intScale;
            ItemSlot::drawItem(window, spriteBatch, itemCount.first, textDrawData.position + pl::Vector2f(itemSize, itemSize) * 0.5f * static_cast<float>(intScale), 1.0f, true, alpha);
            spriteBatch.endDrawing(window);

            // Draw item amount
            if (infoString.drawItemCountNumberWhenOne || !infoString.drawItemCountNumberWhenOne && itemCount.second > 1)
            {
                pl::TextDrawData itemAmountText = {
                    .text = std::to_string(itemCount.second),
                    .position = textDrawData.position + pl::Vector2f(itemSize, itemSize) * 0.85f * static_cast<float>(intScale),
                    .color = pl::Color(255, 255, 255, alpha),
                    .size = textDrawData.size,
                    .centeredX = true,
                    .centeredY = true
                };

                TextDraw::drawText(window, itemAmountText);
            }

            // Offset text draw data
            textDrawData.position.x += (itemSize + textXPadding * 2) * intScale;
            textDrawData.position.y += itemSize * 0.5f * intScale;
            textDrawData.centeredY = true;
        }

        TextDraw::drawText(window, textDrawData);

        // Update y offset
        if (infoString.itemCount.has_value())
        {
            textYOffset += (itemSize + textYPadding) * intScale;
        }
        else
        {
            textYOffset += TextDraw::getTextSize(textDrawData).height + textYPadding * intScale;
        }
    }

    static constexpr float boxXPadding = 28.0f;
    float totalWidth = ((sides[3].width + sides[1].width) * 3 + boxXPadding) * intScale + width;
    float totalHeight = (sides[0].width + sides[2].width) * 3 * intScale + height;

    return pl::Vector2f(totalWidth, totalHeight);
}

bool InventoryGUI::canPutDownItemInArmourInventory(int hoveredIndex)
{
    // Get held item data
    const ItemData& pickedUpItemData = ItemDataLoader::getItemData(pickedUpItem);
    
    // If picked up item is not armour, cannot put down here
    if (pickedUpItemData.armourType < 0)
    {
        return false;
    }

    const ArmourData& heldArmourData = ArmourDataLoader::getArmourData(pickedUpItemData.armourType);

    switch (hoveredIndex)
    {
        case 0: return (heldArmourData.armourWearType == ArmourWearType::Head);
        case 1: return (heldArmourData.armourWearType == ArmourWearType::Chest);
        case 2: return (heldArmourData.armourWearType == ArmourWearType::Feet);
    }

    // Default case
    return false;
}


// -- Hotbar -- //

void InventoryGUI::updateHotbar(float dt, pl::Vector2f mouseScreenPos)
{
    if (InputManager::isControllerActive())
    {
        // If controller is being used, do not update item slots based on mouse position
        mouseScreenPos = pl::Vector2f(0, 0);
    }

    for (int i = 0; i < hotbarItemSlots.size(); i++)
    {
        ItemSlot& hotbarItemSlot = hotbarItemSlots[i];

        bool hotbarSelected = (i == selectedHotbarIndex);

        hotbarItemSlot.update(mouseScreenPos, dt, hotbarSelected);
    }

    // Update item string timer
    hotbarItemStringTimer = std::max(hotbarItemStringTimer - dt, 0.0f);
}

bool InventoryGUI::handleLeftClickHotbar(pl::Vector2f mouseScreenPos)
{
    // Get hovered index
    int hoveredIndex = getHoveredItemSlotIndex(hotbarItemSlots, mouseScreenPos);

    // Set selected index to hovered, as left click has occured
    if (hoveredIndex >= 0)
    {
        selectedHotbarIndex = hoveredIndex;
        handleHotbarItemChange();

        // Interaction with hotbar
        return true;
    }

    // No interaction with hotbar
    return false;
}

void InventoryGUI::handleScrollHotbar(int direction)
{
    selectedHotbarIndex = ((selectedHotbarIndex + direction) % ITEM_BOX_PER_ROW + ITEM_BOX_PER_ROW) % ITEM_BOX_PER_ROW;

    handleHotbarItemChange();
}

void InventoryGUI::setHotbarSelectedIndex(int index)
{
    selectedHotbarIndex = (index % ITEM_BOX_PER_ROW + ITEM_BOX_PER_ROW) % ITEM_BOX_PER_ROW;

    handleHotbarItemChange();   
}

ObjectType InventoryGUI::getHotbarSelectedObject(InventoryData& inventory)
{
    // Get item
    const std::optional<ItemCount>& itemSlot = inventory.getItemSlotData(selectedHotbarIndex);

    if (!itemSlot.has_value())
        return -1;
    
    ItemType itemType = itemSlot.value().first;

    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    return itemData.placesObjectType;
}

ToolType InventoryGUI::getHotbarSelectedTool(InventoryData& inventory)
{
    // Get item
    const std::optional<ItemCount>& itemSlot = inventory.getItemSlotData(selectedHotbarIndex);

    if (!itemSlot.has_value())
        return -1;
    
    ItemType itemType = itemSlot.value().first;

    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    return itemData.toolType;
}

bool InventoryGUI::hotbarItemPlacesLand(InventoryData& inventory)
{
    // Get item
    const std::optional<ItemCount>& itemSlot = inventory.getItemSlotData(selectedHotbarIndex);

    if (!itemSlot.has_value())
        return false;
    
    ItemType itemType = itemSlot.value().first;

    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    return itemData.placesLand;
}

void InventoryGUI::subtractHotbarItem(InventoryData& inventory)
{
    inventory.takeItemAtIndex(selectedHotbarIndex, 1);
}

void InventoryGUI::handleHotbarItemChange()
{
    // Set timer to max
    hotbarItemStringTimer = HOTBAR_ITEM_STRING_OPAQUE_TIME + HOTBAR_ITEM_STRING_FADE_TIME;
}

void InventoryGUI::drawHotbar(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, pl::Vector2f mouseScreenPos, InventoryData& inventory)
{
    // Get resolution
    const pl::Vector2<uint32_t>& resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    std::string selectedItemName = "";

    for (int i = 0; i < hotbarItemSlots.size(); i++)
    {
        const std::optional<ItemCount>& itemSlotData = inventory.getItemSlotData(i);

        bool selected = (i == selectedHotbarIndex);

        ItemSlot& itemSlot = hotbarItemSlots[i];

        if (itemSlotData.has_value())
        {
            const ItemCount& itemCount = itemSlotData.value();

            itemSlot.draw(window, spriteBatch, itemCount.first, itemCount.second, false, selected, std::nullopt, &inventory);

            // Set item name string if selected
            if (selected)
            {
                ItemType itemType = itemCount.first;
                selectedItemName = ItemDataLoader::getItemData(itemType).getDisplayName();
            }
        }
        else
        {
            // Draw blank item box
            itemSlot.draw(window, spriteBatch, std::nullopt, std::nullopt, false, selected);
        }
    }

    spriteBatch.endDrawing(window);

    // Draw item name string
    if (hotbarItemStringTimer > 0)
    {
        // Fade string color if required
        float alpha = std::min(hotbarItemStringTimer / HOTBAR_ITEM_STRING_FADE_TIME, 1.0f) * 255.0f;

        // Draw text
        TextDraw::drawText(window, {
            .text = selectedItemName,
            .position = (hotbarItemSlots[0].getPosition() + pl::Vector2f(0, itemBoxSize)) * intScale,
            .color = pl::Color(255, 255, 255, alpha),
            .size = 24 * static_cast<unsigned int>(intScale),
            .outlineColor = pl::Color(46, 34, 47, alpha),
            .outlineThickness = 2 * static_cast<unsigned int>(intScale)
        });
    }
}

// -- Chest -- //
void InventoryGUI::chestOpened(InventoryData* chestData)
{
    createChestItemSlots(chestData);
}

void InventoryGUI::chestClosed()
{
    // Delete chest item slots GUI elements
    chestItemSlots.clear();

    // Handle controller navigation
    if (controllerSelectedItemSlots = &chestItemSlots)
    {
        controllerSelectedItemSlots = nullptr;
        controllerSelectedSlotIndex = 0;
    }
}

void InventoryGUI::createChestItemSlots(InventoryData* chestData)
{
    if (chestData == nullptr)
        return;
    
    chestItemSlots.clear();

    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());

    static const float xStart = itemBoxPadding + (ITEM_BOX_PER_ROW + 4) * itemBoxSize;

    pl::Vector2f chestItemBoxPosition = pl::Vector2f(xStart, itemBoxPadding);

    int currentRowIndex = 0;
    int currentColumnIndex = 0;
    for (int itemIndex = 0; itemIndex < chestData->getSize(); itemIndex++)
    {
        ItemSlot chestItemSlot(chestItemBoxPosition, itemBoxSize);

        chestItemSlots.push_back(chestItemSlot);

        // Increment box position
        chestItemBoxPosition.x += itemBoxSize + itemBoxSpacing;

        currentRowIndex++;
        currentColumnIndex++;
        if (currentRowIndex >= CHEST_BOX_PER_ROW || chestItemBoxPosition.x + itemBoxSize + itemBoxSpacing >= resolution.x)
        {
            // Store current width / count of chest slot row
            currentChestBoxPerRow = currentColumnIndex;

            currentColumnIndex = 0;

            // Increment to next row
            currentRowIndex = 0;
            chestItemBoxPosition.x = xStart;
            chestItemBoxPosition.y += itemBoxSize + itemBoxSpacing;
        }
    }
}

void InventoryGUI::inventoryChestItemQuickTransfer(Game& game, pl::Vector2f mouseScreenPos, unsigned int amount, InventoryData& inventory, InventoryData& chestData)
{
    InventoryData* hoveredInventory = &inventory;
    InventoryData* destinationInventory = &chestData;

    int itemHovered = getHoveredItemSlotIndex(inventoryItemSlots, mouseScreenPos);
    if (itemHovered < 0)
    {
        itemHovered = getHoveredItemSlotIndex(chestItemSlots, mouseScreenPos);
        hoveredInventory = &chestData;
        destinationInventory = &inventory;
    }

    if (itemHovered < 0)
        return;
    
    // Get item data from source inventory
    std::optional<ItemCount>& itemSlotData = hoveredInventory->getItemSlotData(itemHovered);

    if (!itemSlotData.has_value())
        return;
    
    ItemCount& itemCount = itemSlotData.value();

    // Attempt to add amount of item to destination inventory
    int amountTransfered = destinationInventory->addItem(itemCount.first, std::min(itemCount.second, amount));

    // Subtract amount from source inventory
    hoveredInventory->takeItemAtIndex(itemHovered, amountTransfered);

    game.openedChestDataModified();
}


// -- Shop -- //
void InventoryGUI::shopOpened(ShopInventoryData& shopData)
{
    openShopData = shopData;

    // Create item slots
    chestOpened(&shopData);
}

void InventoryGUI::shopClosed()
{
    openShopData = std::nullopt;
    chestClosed();
}

bool InventoryGUI::attemptPurchaseItem(InventoryData& inventory, int shopIndex)
{
    const std::optional<ItemCount>& shopItemSlot = openShopData->getItemSlotData(shopIndex);
    if (!shopItemSlot.has_value())
    {
        std::cout << "Shop has no item in slot\n";
        return false;
    }

    // If item is picked up, ensure item is same and can fit in hand
    if (isItemPickedUp)
    {
        if (pickedUpItem != shopItemSlot->first)
        {
            return false;
        }

        const ItemData& pickedUpItemData = ItemDataLoader::getItemData(pickedUpItem);

        if (pickedUpItemCount + shopItemSlot->second > pickedUpItemData.maxStackSize)
        {
            return false;
        }
    }
    
    // Ensure can afford
    int currencyTotal = inventory.getCurrencyValueTotal();

    int price = std::floor(openShopData->getItemBuyPrice(shopItemSlot->first) * shopItemSlot->second);

    if (currencyTotal < price)
    {
        return false;
    }

    // Can afford, take money
    inventory.takeCurrencyValueItems(price);

    return true;
}

bool InventoryGUI::attemptSellItemHeld(InventoryData& inventory)
{
    int currencyToGive = std::floor(openShopData->getItemSellPrice(pickedUpItem) * pickedUpItemCount);

    if (inventory.addCurrencyValueItems(currencyToGive) >= currencyToGive)
    {
        // Return false if no currency was given
        return false;
    }

    // Take held item
    isItemPickedUp = false;
    pickedUpItem = -1;
    pickedUpItemCount = 0;

    // Return true if some currency / all was given
    return true;
}


// -- Popups -- //
void InventoryGUI::updateItemPopups(float dt)
{
    for (auto popupIter = itemPopups.begin(); popupIter != itemPopups.end();)
    {
        popupIter->timeAlive += dt;

        if (popupIter->timeAlive >= POPUP_LIFETIME)
        {
            popupIter = itemPopups.erase(popupIter);
            continue;
        }

        popupIter++;
    }
}

void InventoryGUI::pushItemPopup(const ItemCount& itemCount, bool notEnoughSpace, std::optional<std::string> textOverride)
{
    if (itemCount.second <= 0)
    {
        return;
    }

    bool addedToExisting = false;

    if (!textOverride.has_value())
    {
        // Add to item popup if same item is already in popups
        for (auto iter = itemPopups.begin(); iter != itemPopups.end();)
        {
            if (iter->notEnoughSpace == notEnoughSpace)
            {
                // Only add to popup if "not enough space" labels are the same

                if (itemCount.first == iter->itemCount.first)
                {
                    // Popup of same item type found
                    ItemPopup popupToAddTo = *iter;
                    popupToAddTo.itemCount.second += itemCount.second;
                    popupToAddTo.timeAlive = 0.0f;

                    iter = itemPopups.erase(iter);
                    itemPopups.push_back(popupToAddTo);

                    addedToExisting = true;

                    break;
                }
            }

            iter++;
        }
    }

    if (!addedToExisting)
    {
        // Item type popup is not in popups, so add new
        ItemPopup itemPopup;
        itemPopup.itemCount = itemCount;
        itemPopup.notEnoughSpace = notEnoughSpace;
        itemPopup.textOverride = textOverride;

        itemPopups.push_back(itemPopup);
    }

    if (itemPopups.size() > POPUP_MAX_COUNT)
    {
        itemPopups.erase(itemPopups.begin());
    }
}

void InventoryGUI::drawItemPopups(pl::RenderTarget& window, float gameTime)
{
    if (itemPopups.size() <= 0)
        return;

    pl::Framebuffer popupTexture;
    popupTexture.create(window.getWidth(), window.getHeight());
    popupTexture.clear(pl::Color(0, 0, 0, 0));

    int intScale = ResolutionHandler::getResolutionIntegerScale();

    pl::Vector2f popupPos(itemBoxPadding * intScale, itemBoxPadding * intScale);

    for (const ItemPopup& itemPopup : itemPopups)
    {
        const ItemData& itemData = ItemDataLoader::getItemData(itemPopup.itemCount.first);

        ItemInfoString infoString;
        infoString.itemCount = itemPopup.itemCount;
        infoString.string = itemData.getDisplayName();
        infoString.color = itemData.getNameColor(gameTime);

        if (itemPopup.textOverride.has_value())
        {
            infoString.string = itemPopup.textOverride.value();
            infoString.drawItemCountNumberWhenOne = false;
        }
        else if (itemPopup.notEnoughSpace)
        {
            infoString.string += " - Inventory Full";
            infoString.color = pl::Color(232, 59, 59);
        }

        infoString.size = 20;

        // Calculate alpha
        int alpha = std::max(std::min((POPUP_LIFETIME - itemPopup.timeAlive) / POPUP_FADE_TIME, 1.0f), 0.0f) * 255;

        float flashAmount = std::max(POPUP_FLASH_TIME - itemPopup.timeAlive, 0.0f) / POPUP_FLASH_TIME;

        pl::Vector2f boxSize = drawInfoBox(popupTexture, popupPos, {infoString}, alpha, flashAmount);
        popupPos.y += boxSize.y + (itemBoxSpacing + 6) * intScale;
    }

    pl::VertexArray popupRect;
    popupRect.addQuad(pl::Rect<float>(0, window.getHeight() - popupPos.y - 9 * intScale, popupTexture.getWidth(), popupTexture.getHeight()),
        pl::Color(), pl::Rect<float>(0, popupTexture.getHeight(), popupTexture.getWidth(), -popupTexture.getHeight()));

    window.draw(popupRect, *Shaders::getShader(ShaderType::Default), &popupTexture.getTexture(), pl::BlendMode::Alpha);
}


// -- Controller navigation -- //
bool InventoryGUI::handleControllerInput(Game& game, NetworkHandler& networkHandler, InventoryData& inventory, InventoryData& armourInventory, InventoryData* chestData)
{
    if (!InputManager::isControllerActive())
    {
        return false;
    }

    if (controllerSelectedItemSlots == nullptr)
    {
        controllerSelectedItemSlots = &inventoryItemSlots;
    }

    if (controllerSelectedSlotIndex >= controllerSelectedItemSlots->size())
    {
        controllerSelectedSlotIndex = 0;
    }

    if (InputManager::isActionJustActivated(InputAction::UI_RIGHT))
    {
        bool transferredSlots = false;
        if (controllerSelectedItemSlots == &inventoryItemSlots)
        {
            if (controllerSelectedSlotIndex == ITEM_BOX_PER_ROW - 1 && isItemPickedUp)
            {
                controllerSelectedItemSlots = &binItemSlot;
                controllerSelectedSlotIndex = 0;
                transferredSlots = true;
            }
            else if (controllerSelectedSlotIndex % ITEM_BOX_PER_ROW >= ITEM_BOX_PER_ROW - 1)
            {
                controllerSelectedItemSlots = &armourItemSlots;
                controllerSelectedSlotIndex = std::clamp(controllerSelectedSlotIndex / ITEM_BOX_PER_ROW - 1, 0, static_cast<int>(armourItemSlots.size()) - 1);
                transferredSlots = true;
            }
        }
        else if (controllerSelectedItemSlots == &armourItemSlots || controllerSelectedItemSlots == &binItemSlot)
        {
            if (chestItemSlots.size() > 0)
            {
                controllerSelectedItemSlots = &chestItemSlots;
                controllerSelectedSlotIndex = 0;
            }
            transferredSlots = true;
        }
        else if (controllerSelectedItemSlots == &recipeItemSlots)
        {
            if (controllerSelectedSlotIndex % ITEM_BOX_PER_ROW == ITEM_BOX_PER_ROW - 1 || controllerSelectedSlotIndex == recipeItemSlots.size() - 1)
            {
                int recipePageCount = static_cast<int>(std::floor((availableRecipes.size() - 1) / (RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW))) + 1;
                if (recipePageCount > 1)
                {
                    recipeCurrentPage = ((recipeCurrentPage + 1) % recipePageCount + recipePageCount) % recipePageCount;
                    createRecipeItemSlots(inventory);
                    controllerSelectedSlotIndex = std::clamp(controllerSelectedSlotIndex - (ITEM_BOX_PER_ROW - 1), 0, static_cast<int>(recipeItemSlots.size()) - 1);
                }
                transferredSlots = true;
            }
        }

        if (!transferredSlots)
        {
            controllerSelectedSlotIndex = std::min(controllerSelectedSlotIndex + 1, static_cast<int>(controllerSelectedItemSlots->size()) - 1);
        }
    }
    if (InputManager::isActionJustActivated(InputAction::UI_LEFT))
    {
        bool transferredSlots = false;
        if (controllerSelectedItemSlots == &armourItemSlots)
        {
            controllerSelectedItemSlots = &inventoryItemSlots;
            controllerSelectedSlotIndex = std::clamp((controllerSelectedSlotIndex + 2) * ITEM_BOX_PER_ROW - 1, 0, static_cast<int>(inventoryItemSlots.size()) - 1);
            transferredSlots = true;
        }
        else if (controllerSelectedItemSlots == &chestItemSlots)
        {
            if (controllerSelectedSlotIndex % currentChestBoxPerRow == 0)
            {
                controllerSelectedItemSlots = &armourItemSlots;
                controllerSelectedSlotIndex = 0;
                transferredSlots = true;
            }
        }
        else if (controllerSelectedItemSlots == &recipeItemSlots)
        {
            if (controllerSelectedSlotIndex % ITEM_BOX_PER_ROW == 0)
            {
                int recipePageCount = static_cast<int>(std::floor((availableRecipes.size() - 1) / (RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW))) + 1;
                if (recipePageCount > 1)
                {
                    recipeCurrentPage = ((recipeCurrentPage - 1) % recipePageCount + recipePageCount) % recipePageCount;
                    createRecipeItemSlots(inventory);
                    controllerSelectedSlotIndex = std::min(controllerSelectedSlotIndex + ITEM_BOX_PER_ROW - 1, static_cast<int>(recipeItemSlots.size()) - 1);
                }
                transferredSlots = true;
            }
        }
        else if (controllerSelectedItemSlots == &binItemSlot)
        {
            controllerSelectedItemSlots = &inventoryItemSlots;
            controllerSelectedSlotIndex = ITEM_BOX_PER_ROW - 1;
            transferredSlots = true;
        }

        if (!transferredSlots)
        {
            controllerSelectedSlotIndex = std::max(controllerSelectedSlotIndex - 1, 0);
        }
    }
    if (InputManager::isActionJustActivated(InputAction::UI_UP))
    {
        if (controllerSelectedItemSlots == &inventoryItemSlots)
        {
            if (controllerSelectedSlotIndex - ITEM_BOX_PER_ROW >= 0)
            {
                controllerSelectedSlotIndex -= ITEM_BOX_PER_ROW;
            }
        }
        else if (controllerSelectedItemSlots == &armourItemSlots)
        {
            controllerSelectedSlotIndex = std::max(controllerSelectedSlotIndex - 1, 0);
        }
        else if (controllerSelectedItemSlots == &chestItemSlots)
        {
            if (controllerSelectedSlotIndex - currentChestBoxPerRow >= 0)
            {
                controllerSelectedSlotIndex -= currentChestBoxPerRow;
            }
        }
        else if (controllerSelectedItemSlots == &recipeItemSlots)
        {
            if (controllerSelectedSlotIndex - ITEM_BOX_PER_ROW < 0)
            {
                controllerSelectedItemSlots = &inventoryItemSlots;
                controllerSelectedSlotIndex = inventoryItemSlots.size() - (ITEM_BOX_PER_ROW - controllerSelectedSlotIndex);
            }
            else
            {
                controllerSelectedSlotIndex -= ITEM_BOX_PER_ROW;
            }
        }
    }
    if (InputManager::isActionJustActivated(InputAction::UI_DOWN))
    {
        if (controllerSelectedItemSlots == &inventoryItemSlots)
        {
            if (controllerSelectedSlotIndex + ITEM_BOX_PER_ROW < controllerSelectedItemSlots->size())
            {
                controllerSelectedSlotIndex += ITEM_BOX_PER_ROW;
            }
            else
            {
                controllerSelectedItemSlots = &recipeItemSlots;
                int recipePageCount = static_cast<int>(std::floor((availableRecipes.size() - 1) / (RECIPE_MAX_ROWS * ITEM_BOX_PER_ROW))) + 1;
                controllerSelectedSlotIndex = std::min(controllerSelectedSlotIndex % ITEM_BOX_PER_ROW, static_cast<int>(recipeItemSlots.size()) - 1);
            }
        }
        else if (controllerSelectedItemSlots == &armourItemSlots)
        {
            controllerSelectedSlotIndex = std::min(controllerSelectedSlotIndex + 1, static_cast<int>(controllerSelectedItemSlots->size()) - 1);
        }
        else if (controllerSelectedItemSlots == &chestItemSlots)
        {
            if (controllerSelectedSlotIndex + currentChestBoxPerRow < controllerSelectedItemSlots->size())
            {
                controllerSelectedSlotIndex += currentChestBoxPerRow;
            }
        }
        else if (controllerSelectedItemSlots == &recipeItemSlots)
        {
            if (controllerSelectedSlotIndex + ITEM_BOX_PER_ROW < controllerSelectedItemSlots->size())
            {
                controllerSelectedSlotIndex += ITEM_BOX_PER_ROW;
            }
        }
    }

    if (InputManager::isActionJustActivated(InputAction::UI_CONFIRM))
    {
        handleLeftClick(game, pl::Vector2f(0, 0), InputManager::isActionActive(InputAction::UI_SHIFT), networkHandler, inventory, armourInventory, chestData);
        return true;
    }
    if (InputManager::isActionJustActivated(InputAction::UI_CONFIRM_OTHER))
    {
        handleRightClick(game, pl::Vector2f(0, 0), InputManager::isActionActive(InputAction::UI_SHIFT), networkHandler, inventory, armourInventory, chestData);
        return true;
    }

    return false;
}


// -- Misc -- //
bool InventoryGUI::canQuickTransfer(pl::Vector2f mouseScreenPos, bool shiftMode, InventoryData& inventory, InventoryData* chestData)
{
    if (!shiftMode)
        return false;
    
    if (chestData == nullptr)
        return false;
    
    InventoryData* hoveredInventory = &inventory;
    InventoryData* toTransferInventory = chestData;
    
    int itemHovered = getHoveredItemSlotIndex(inventoryItemSlots, mouseScreenPos);
    if (itemHovered < 0)
    {
        itemHovered = getHoveredItemSlotIndex(chestItemSlots, mouseScreenPos);
        hoveredInventory = chestData;
        toTransferInventory = &inventory;
    }

    // Not hovered over inventory / chest
    if (itemHovered < 0)
    {
        return false;
    }

    std::optional<ItemCount>& itemSlotData = hoveredInventory->getItemSlotData(itemHovered);

    // No item in hovered slot
    if (!itemSlotData.has_value())
    {
        return false;
    }

    // Check available space in inventory to transfer to
    int spaceToAdd = toTransferInventory->addItem(itemSlotData->first, itemSlotData->second, false, false, false);
    if (spaceToAdd <= 0)
    {
        return false;
    }
        
    return true;
}