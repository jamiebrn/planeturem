#include "GUI/InventoryGUI.hpp"

int InventoryGUI::itemBoxSize = 75;
int InventoryGUI::itemBoxSpacing = 10;
int InventoryGUI::itemBoxPadding = 10;

bool InventoryGUI::isItemPickedUp = false;
ItemType InventoryGUI::pickedUpItem;
int InventoryGUI::pickedUpItemCount;

std::unordered_map<std::string, int> InventoryGUI::previous_nearbyCraftingStationLevels;
std::unordered_map<ItemType, unsigned int> InventoryGUI::previous_inventoryItemCount;
std::vector<int> InventoryGUI::availableRecipes;

std::vector<ItemSlot> InventoryGUI::inventoryItemSlots;
std::array<ItemSlot, InventoryGUI::ITEM_BOX_PER_ROW> InventoryGUI::hotbarItemSlots;
std::vector<ItemSlot> InventoryGUI::recipeItemSlots;
std::vector<ItemSlot> InventoryGUI::chestItemSlots;

int InventoryGUI::selectedRecipe = 0;

int InventoryGUI::selectedHotbarIndex = 0;

std::vector<ItemPopup> InventoryGUI::itemPopups;

AnimatedTexture InventoryGUI::binAnimation;
float InventoryGUI::binScale = 1.0f;

float InventoryGUI::hotbarItemStringTimer = 0.0f;

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
    sf::Vector2f itemBoxPosition = sf::Vector2f(itemBoxPadding, itemBoxPadding);

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
}

void InventoryGUI::initialiseHotbar()
{
    sf::Vector2f itemBoxPosition = sf::Vector2f(itemBoxPadding, itemBoxPadding);

    for (int itemIndex = 0; itemIndex < hotbarItemSlots.size(); itemIndex++)
    {
        hotbarItemSlots[itemIndex] = ItemSlot(itemBoxPosition, itemBoxSize);

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
    for (int i = 0; i < availableRecipes.size(); i++)
    {
        ItemSlot recipeItemSlot(sf::Vector2f(xPos, yPos), itemBoxSize);

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
}

void InventoryGUI::updateInventory(sf::Vector2f mouseScreenPos, float dt, InventoryData* chestData)
{
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
    for (ItemSlot& itemSlot : inventoryItemSlots)
    {
        itemSlot.update(mouseScreenPos, dt);
    }

    // Update recipe item slots
    for (int i = 0; i < recipeItemSlots.size(); i++)
    {
        ItemSlot& itemSlot = recipeItemSlots[i];

        bool selected = (i == selectedRecipe);

        itemSlot.update(mouseScreenPos, dt, selected);
    }

    // Update chest item slots
    for (ItemSlot& itemSlot : chestItemSlots)
    {
        itemSlot.update(mouseScreenPos, dt);
    }
}

void InventoryGUI::handleLeftClick(sf::Vector2f mouseScreenPos, bool shiftMode, InventoryData& inventory, InventoryData* chestData)
{
    if (isItemPickedUp)
    {
        putDownItem(mouseScreenPos, inventory, chestData);
    }
    else
    {
        // Pickup max items possible
        if (canQuickTransfer(mouseScreenPos, shiftMode, inventory, chestData))
        {
            inventoryChestItemQuickTransfer(mouseScreenPos, 999999999, inventory, *chestData);
        }
        else
        {
            pickUpItem(mouseScreenPos, 999999999, inventory, chestData);
        }
    }

    int recipeClicked = getHoveredRecipe(mouseScreenPos);
    if (recipeClicked >= 0)
    {
        // If clicked on recipe is selected, attempt to craft item
        if (recipeClicked == selectedRecipe)
        {
            craftSelectedRecipe(inventory);
        }
        else
        {
            // Change selected recipe to recipe clicked on
            selectedRecipe = recipeClicked;
        }
    }
}

void InventoryGUI::handleRightClick(sf::Vector2f mouseScreenPos, bool shiftMode, InventoryData& inventory, InventoryData* chestData)
{
    if (canQuickTransfer(mouseScreenPos, shiftMode, inventory, chestData))
    {
        inventoryChestItemQuickTransfer(mouseScreenPos, 1, inventory, *chestData);
    }
    else
    {
        pickUpItem(mouseScreenPos, 1, inventory, chestData);
    }
}

bool InventoryGUI::handleScroll(sf::Vector2f mouseScreenPos, int direction)
{
    if (!isMouseOverUI(mouseScreenPos))
        return false;
    
    if (availableRecipes.size() > 0)
    {
        int recipeCount = availableRecipes.size();
        selectedRecipe = ((selectedRecipe + direction) % recipeCount + recipeCount) % recipeCount;
    }

    return true;
}

void InventoryGUI::pickUpItem(sf::Vector2f mouseScreenPos, unsigned int amount, InventoryData& inventory, InventoryData* chestData)
{
    // Get item selected at mouse
    int itemIndex = getInventoryHoveredIndex(mouseScreenPos);
    int chestHoveredItemIndex = getHoveredChestIndex(mouseScreenPos);

    // No valid item selected
    if (itemIndex < 0 && chestHoveredItemIndex < 0)
        return;
    
    InventoryData* hoveredInventory = &inventory;
    if (chestHoveredItemIndex >= 0 && chestData != nullptr)
    {
        hoveredInventory = chestData;
        itemIndex = chestHoveredItemIndex;
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
            hoveredInventory->takeItemAtIndex(itemIndex, amountPickedUp);
        }
    }
    else
    {
        // Create new stack in hand
        isItemPickedUp = true;
        pickedUpItem = itemCount.first;
        pickedUpItemCount = std::min(itemCount.second, amount);

        // Take from inventory / chest
        hoveredInventory->takeItemAtIndex(itemIndex, amount);
    }
}

void InventoryGUI::putDownItem(sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData* chestData)
{
    if (!isItemPickedUp)
        return;

    // If bin selected, delete item and return
    if (isBinSelected(mouseScreenPos))
    {
        isItemPickedUp = false;
        pickedUpItem = -1;
        pickedUpItemCount = 0;
        return;
    }

    // Get item selected at mouse
    int chestHoveredItemIndex = getHoveredChestIndex(mouseScreenPos);
    int itemIndex = getInventoryHoveredIndex(mouseScreenPos);

    // No valid item selected
    if (itemIndex < 0 && chestHoveredItemIndex < 0)
        return;

    InventoryData* hoveredInventory = &inventory;
    if (chestHoveredItemIndex >= 0 && chestData != nullptr)
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

        return;
    }

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

int InventoryGUI::getInventoryHoveredIndex(sf::Vector2f mouseScreenPos)
{
    for (int itemIndex = 0; itemIndex < inventoryItemSlots.size(); itemIndex++)
    {
        ItemSlot& itemSlot = inventoryItemSlots[itemIndex];

        if (itemSlot.isHovered())
        {
            return itemIndex;
        }
    }

    // Default case
    return -1;
}

int InventoryGUI::getHoveredRecipe(sf::Vector2f mouseScreenPos)
{
    for (int itemIndex = 0; itemIndex < recipeItemSlots.size(); itemIndex++)
    {
        ItemSlot& itemSlot = recipeItemSlots[itemIndex];

        if (itemSlot.isHovered())
        {
            return itemIndex;
        }
    }

    // Default case
    return -1;
}

int InventoryGUI::getHoveredChestIndex(sf::Vector2f mouseScreenPos)
{    
    for (int itemIndex = 0; itemIndex < chestItemSlots.size(); itemIndex++)
    {
        ItemSlot& itemSlot = chestItemSlots[itemIndex];

        if (itemSlot.isHovered())
        {
            return itemIndex;
        }
    }

    // Default case
    return -1;
}

int InventoryGUI::getHotbarHoveredIndex(sf::Vector2f mouseScreenPos)
{
    for (int itemIndex = 0; itemIndex < hotbarItemSlots.size(); itemIndex++)
    {
        ItemSlot& itemSlot = hotbarItemSlots[itemIndex];

        if (itemSlot.isHovered())
        {
            return itemIndex;
        }
    }

    // Default case
    return -1;
}

bool InventoryGUI::isBinSelected(sf::Vector2f mouseScreenPos)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
     
    sf::Vector2f binPos = sf::Vector2f(itemBoxPadding, itemBoxPadding) * intScale + sf::Vector2f(itemBoxSize + itemBoxSpacing, 0) * static_cast<float>(ITEM_BOX_PER_ROW) * intScale;
    
    // Add offset due to sprite size
    binPos += sf::Vector2f(0, 4) * 3.0f * intScale;

    CollisionRect binCollisionRect;
    binCollisionRect.x = binPos.x;
    binCollisionRect.y = binPos.y;
    binCollisionRect.width = 16.0f * 3 * intScale;
    binCollisionRect.height = 16.0f * 3 * intScale;

    return binCollisionRect.isPointInRect(mouseScreenPos.x, mouseScreenPos.y);
}

bool InventoryGUI::isInventorySelected(sf::Vector2f mouseScreenPos)
{
    return (getInventoryHoveredIndex(mouseScreenPos) >= 0);
}

bool InventoryGUI::isCraftingSelected(sf::Vector2f mouseScreenPos)
{
    return (getHoveredRecipe(mouseScreenPos) >= 0);
}

void InventoryGUI::craftSelectedRecipe(InventoryData& inventory)
{
    // Get recipe data
    int recipeIdx = availableRecipes[selectedRecipe];
    const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipeIdx];

    // If item in hand and is not item to be crafted, do not craft recipe (as won't be able to pick up crafted item)
    if (isItemPickedUp)
    {
        const ItemData& pickedUpItemData = ItemDataLoader::getItemData(pickedUpItem);

        if ((pickedUpItem != recipeData.product || pickedUpItemCount >= pickedUpItemData.maxStackSize))
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

bool InventoryGUI::isMouseOverUI(sf::Vector2f mouseScreenPos)
{
    return (isInventorySelected(mouseScreenPos) || isCraftingSelected(mouseScreenPos) || isBinSelected(mouseScreenPos) || 
        (getHoveredChestIndex(mouseScreenPos) >= 0));
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
                // if (inventoryItemCount[itemRequired.first] >= itemRequired.second)
                // {
                //     hasItems = true;
                //     break;
                // }
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
        if (availableRecipes.size() > 0)
        {
            selectedRecipe = (selectedRecipe % availableRecipes.size() + availableRecipes.size()) % availableRecipes.size();
        }
        else
        {
            selectedRecipe = 0;
        }

        createRecipeItemSlots(inventory);
    }
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

ObjectType InventoryGUI::getHeldObjectType(InventoryData& inventory)
{
    if (isItemPickedUp)
    {
        ObjectType placeObjectType = ItemDataLoader::getItemData(pickedUpItem).placesObjectType;
        return placeObjectType;
    }

    // Get from hotbar as no item is picked up
    return getHotbarSelectedObject(inventory);
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

void InventoryGUI::placeHeldObject(InventoryData& inventory)
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
    placeHotbarObject(inventory);
}

bool InventoryGUI::heldItemPlacesLand(InventoryData& inventory)
{
    if (isItemPickedUp)
    {
        const ItemData& itemData = ItemDataLoader::getItemData(pickedUpItem);
        return itemData.placesLand;
    }

    // Get from hotbar
    return hotbarItemPlacesLand(inventory);
}

void InventoryGUI::draw(sf::RenderTarget& window, float gameTime, sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData* chestData)
{
    // Get intscale
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Get currently hovered over item
    int hoveredItemIndex = getInventoryHoveredIndex(mouseScreenPos);

    // Get currently hovered recipe
    int hoveredRecipeIndex = getHoveredRecipe(mouseScreenPos);

    // Draw inventory data
    for (int itemIdx = 0; itemIdx < inventoryItemSlots.size(); itemIdx++)
    {
        const std::optional<ItemCount>& itemSlotData = inventory.getItemSlotData(itemIdx);

        ItemSlot& itemSlot = inventoryItemSlots[itemIdx];

        if (itemSlotData.has_value())
        {
            const ItemCount& itemCount = itemSlotData.value();

            itemSlot.draw(window, itemCount.first, itemCount.second);
        }
        else
        {
            // Draw blank item box
            itemSlot.draw(window);
        }
    }

    // Draw bin
    sf::Vector2f binPosition;
    binPosition.x = itemBoxPadding * intScale + (itemBoxSize + itemBoxSpacing) * static_cast<float>(ITEM_BOX_PER_ROW) * intScale + (16 / 2) * 3 * intScale;
    binPosition.y = itemBoxPadding * intScale + (20 / 2) * 3 * intScale;

    TextureManager::drawSubTexture(window, {
        TextureType::UI,
        binPosition,
        0,
        {3 * binScale * intScale, 3 * binScale * intScale},
        {0.5, 0.5}
    }, binAnimation.getTextureRect());


    // Draw Available recipes
    if (recipeItemSlots.size() > 0)
    {
        // Store recipe index hovered over for drawing
        int hoveredRecipeIdx = -1;

        // Draw recipes
        for (int i = 0; i < recipeItemSlots.size(); i++)
        {
            // Get recipe index
            int recipeIdx = availableRecipes[i % recipeItemSlots.size()];

            // Get recipe data
            const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipeIdx];

            // Get item slot
            ItemSlot& itemSlot = recipeItemSlots[i];

            // If recipe is selected, draw requirements
            bool selected = i == selectedRecipe;
            // if (i == selectedRecipe)
            // {
            //     selected = true;

            //     int requirementXPos = itemSlot.getPosition().x;
            //     int requirementYPos = itemSlot.getPosition().y + itemBoxSize;

            //     for (const auto& itemRequired : recipeData.itemRequirements)
            //     {
            //         // Draw requirement
            //         ItemSlot recipeItem(sf::Vector2f(requirementXPos, requirementYPos), itemBoxSize);

            //         recipeItem.draw(window, itemRequired.first, itemRequired.second, true);

            //         // Increment position
            //         requirementYPos += itemBoxSize + itemBoxSpacing;
            //     }
            // }

            // Draw item box for product
            itemSlot.draw(window, recipeData.product, recipeData.productAmount, false, selected);
            
            // Test whether mouse is over - if so, store recipe index for drawing info later
            if (itemSlot.isHovered())
            {
                hoveredRecipeIdx = recipeIdx;
            }
        }

        // Draw hammer icon
        TextureManager::drawSubTexture(window, {
            TextureType::UI,
            (recipeItemSlots.back().getPosition() + sf::Vector2f(itemBoxSize + itemBoxSpacing, 0)) * intScale,
            0,
            {3 * intScale, 3 * intScale},
        }, sf::IntRect(80, 16, 16, 16));

        // Draw info of recipe hovered over (if any)
        if (hoveredRecipeIdx >= 0 && !isItemPickedUp)
        {
            drawItemInfoBoxRecipe(window, gameTime, hoveredRecipeIdx, mouseScreenPos);
        }
    }


    // Draw Chest
    if (chestData != nullptr)
    {
        for (int itemIdx = 0; itemIdx < chestItemSlots.size(); itemIdx++)
        {
            const std::optional<ItemCount>& itemSlotData = chestData->getItemSlotData(itemIdx);

            ItemSlot& itemSlot = chestItemSlots[itemIdx];

            if (itemSlotData.has_value())
            {
                const ItemCount& itemCount = itemSlotData.value();

                itemSlot.draw(window, itemCount.first, itemCount.second);
            }
            else
            {
                // Draw blank item box
                itemSlot.draw(window);
            }
        }
    }

    // Draw picked up item at cursor
    if (isItemPickedUp)
    {
        // Create dummy item slot at cursor
        ItemSlot pickedUpItemSlot(mouseScreenPos, itemBoxSize, false);
        pickedUpItemSlot.overrideItemScaleMult(1.0f + std::pow(std::sin(gameTime * 3.5f), 2) / 8.0f);
        pickedUpItemSlot.draw(window, pickedUpItem, pickedUpItemCount, true);
    }
    else
    {
        // Draw item info box if no item held in cursor and hovering over item
    
        // If an item is hovered over, draw item info box
        if (hoveredItemIndex >= 0)
        {
            drawItemInfoBox(window, gameTime, hoveredItemIndex, inventory, mouseScreenPos);
        }
        else if (chestData != nullptr)
        {
            // Draw chest hovered item info
            int chestHoveredItemIndex = getHoveredChestIndex(mouseScreenPos);
            if (chestHoveredItemIndex >= 0)
            {
                drawItemInfoBox(window, gameTime, chestHoveredItemIndex, *chestData, mouseScreenPos);
            }
        }
    }
}

sf::Vector2f InventoryGUI::drawItemInfoBox(sf::RenderTarget& window, float gameTime, int itemIndex, InventoryData& inventory, sf::Vector2f mouseScreenPos)
{
    const std::optional<ItemCount>& itemSlot = inventory.getItemSlotData(itemIndex);

    // If no item in slot, do not draw box
    if (!itemSlot.has_value())
        return sf::Vector2f(0, 0);
    
    ItemType itemType = itemSlot.value().first;

    return drawItemInfoBox(window, gameTime, itemType, mouseScreenPos);
}

sf::Vector2f InventoryGUI::drawItemInfoBox(sf::RenderTarget& window, float gameTime, ItemType itemType, sf::Vector2f mouseScreenPos)
{
    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    std::vector<ItemInfoString> infoStrings;

    sf::Color itemNameColor = itemData.nameColor;
    if (itemData.nameColor == sf::Color(0, 0, 0))
    {
        itemNameColor.r = 255.0f * (std::sin(gameTime * 2.0f) + 1) / 2.0f;
        itemNameColor.g = 255.0f * (std::cos(gameTime * 2.0f) + 1) / 2.0f;
        itemNameColor.b = 255.0f * (std::sin(gameTime * 2.0f + 3 * 3.14 / 2) + 1) / 2.0f;
    }

    infoStrings.push_back({itemData.name, 24, itemNameColor});

    if (itemData.placesObjectType >= 0 || itemData.placesLand)
    {
        infoStrings.push_back({"Can be placed", 20});
    }
    else if (itemData.toolType >= 0)
    {
        const ToolData& toolData = ToolDataLoader::getToolData(itemData.toolType);

        if (toolData.damage > 0)
        {
            infoStrings.push_back({std::to_string(toolData.damage) + " damage", 20});
        }
    }

    if (itemData.isMaterial)
    {
        infoStrings.push_back({"Material", 20});
    }

    return drawInfoBox(window, mouseScreenPos + sf::Vector2f(8, 8) * 3.0f * intScale, infoStrings);
}

void InventoryGUI::drawItemInfoBoxRecipe(sf::RenderTarget& window, float gameTime, int recipeIdx, sf::Vector2f mouseScreenPos)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipeIdx];

    sf::Vector2f itemInfoBoxSize = drawItemInfoBox(window, gameTime, recipeData.product, mouseScreenPos);

    std::vector<ItemInfoString> infoStrings;

    infoStrings.push_back({"Requires", 20});

    for (const auto& item : recipeData.itemRequirements)
    {
        ItemInfoString itemRequirement;
        itemRequirement.itemCount = ItemCount(item.first, item.second);

        const ItemData& itemData = ItemDataLoader::getItemData(item.first);
        itemRequirement.string = itemData.name;
        itemRequirement.size = 20;

        itemRequirement.color = sf::Color(232, 59, 59);
        if (previous_inventoryItemCount.count(item.first) >= 0)
        {
            if (previous_inventoryItemCount[item.first] >= item.second)
            {
                itemRequirement.color = sf::Color(255, 255, 255);
            }
        }

        infoStrings.push_back(itemRequirement);
    }

    drawInfoBox(window, mouseScreenPos + sf::Vector2f(8, 8 + 6) * 3.0f * intScale + sf::Vector2f(0, itemInfoBoxSize.y), infoStrings);
}

sf::Vector2f InventoryGUI::drawInfoBox(sf::RenderTarget& window, sf::Vector2f position, const std::vector<ItemInfoString>& infoStrings, int alpha)
{
    static const std::array<sf::IntRect, 4> sides = {
        sf::IntRect(20, 80, 1, 3), // top
        sf::IntRect(22, 84, 3, 1), // right
        sf::IntRect(20, 86, 1, 3), // bottom
        sf::IntRect(16, 84, 3, 1)  // left
    };
    static const std::array<sf::IntRect, 4> corners = {
        sf::IntRect(16, 80, 3, 3), // top left
        sf::IntRect(22, 80, 3, 3), // top right
        sf::IntRect(22, 86, 3, 3),  // bottom right
        sf::IntRect(16, 86, 3, 3) // bottom left
    };
    static const sf::IntRect centre(20, 84, 1, 1);

    int intScale = ResolutionHandler::getResolutionIntegerScale();

    // Get size of box
    int width = 0;
    int height = 0;
    static constexpr int textXPadding = 5;
    static constexpr int textYPadding = 10;

    static constexpr int itemSize = 16 * 3;

    for (const ItemInfoString& infoString : infoStrings)
    {
        TextDrawData textDrawData = {
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
    SpriteBatch spriteBatch;

    sf::Vector2f scale(3 * intScale, 3 * intScale);

    sf::Color colour(255, 255, 255, alpha);

    // Draw corners
    spriteBatch.draw(window, {TextureType::UI, position, 0, scale, {0, 0}, colour}, corners[0]);
    spriteBatch.draw(window, {TextureType::UI, position + sf::Vector2f(width + sides[1].width * scale.y, 0), 0, scale, {0, 0}, colour}, corners[1]);
    spriteBatch.draw(window, {TextureType::UI,
        position + sf::Vector2f(width + sides[1].width * scale.x, height + sides[0].height * scale.y), 0, scale, {0, 0}, colour}, corners[2]);
    spriteBatch.draw(window, {TextureType::UI, position + sf::Vector2f(0, height + sides[0].height * scale.y), 0, scale, {0, 0}, colour}, corners[3]);

    // Draw sides
    spriteBatch.draw(window, {TextureType::UI,
        position + sf::Vector2f(sides[3].width * scale.x, 0), 0, sf::Vector2f(width, scale.y), {0, 0}, colour}, sides[0]);
    spriteBatch.draw(window, {TextureType::UI,
        position + sf::Vector2f(sides[3].width * scale.x + width, sides[0].height * scale.y), 0, sf::Vector2f(scale.x, height), {0, 0}, colour}, sides[1]);
    spriteBatch.draw(window, {TextureType::UI,
        position + sf::Vector2f(sides[3].width * scale.x, sides[0].height * scale.y + height), 0, sf::Vector2f(width, scale.y), {0, 0}, colour}, sides[2]);
    spriteBatch.draw(window, {TextureType::UI,
        position + sf::Vector2f(0, sides[0].height * scale.x), 0, sf::Vector2f(scale.x, height), {0, 0}, colour}, sides[3]);

    // Draw centre
    spriteBatch.draw(window, {TextureType::UI,
        position + sf::Vector2f(sides[3].width * scale.x, sides[0].height * scale.y), 0,
        sf::Vector2f(width,  height), {0, 0}, colour}, centre);

    spriteBatch.endDrawing(window);

    // Draw text
    int textYOffset = 0;
    static constexpr int textYShift = 6;

    for (int i = 0; i < infoStrings.size(); i++)
    {
        const ItemInfoString& infoString = infoStrings[i];

        TextDrawData textDrawData = {
            .text = infoString.string,
            .position = position + sf::Vector2f(sides[3].width * 3 * intScale + textXPadding * intScale,
                sides[0].height * 3 * intScale + textYOffset - textYShift * intScale),
            .colour = sf::Color(infoString.color.r, infoString.color.g, infoString.color.b, alpha),
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
            ItemSlot::drawItem(window, itemCount.first, textDrawData.position + sf::Vector2f(itemSize, itemSize) * 0.5f * static_cast<float>(intScale), 1.0f, true, alpha);

            // Draw item amount
            TextDrawData itemAmountText = {
                .text = std::to_string(itemCount.second),
                .position = textDrawData.position + sf::Vector2f(itemSize, itemSize) * 0.85f * static_cast<float>(intScale),
                .colour = sf::Color(textDrawData.colour.r, textDrawData.colour.g, textDrawData.colour.b, alpha),
                .size = textDrawData.size,
                .centeredX = true,
                .centeredY = true
            };

            TextDraw::drawText(window, itemAmountText);

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

    float totalWidth = (sides[3].width + sides[1].width) * 3 * intScale + width;
    float totalHeight = (sides[0].width + sides[2].width) * 3 * intScale + height;

    return sf::Vector2f(totalWidth, totalHeight);
}


// -- Hotbar -- //

void InventoryGUI::updateHotbar(float dt, sf::Vector2f mouseScreenPos)
{
    for (int i = 0; i < hotbarItemSlots.size(); i++)
    {
        ItemSlot& hotbarItemSlot = hotbarItemSlots[i];

        bool hotbarSelected = (i == selectedHotbarIndex);

        hotbarItemSlot.update(mouseScreenPos, dt, hotbarSelected);
    }

    // Update item string timer
    hotbarItemStringTimer = std::max(hotbarItemStringTimer - dt, 0.0f);
}

bool InventoryGUI::handleLeftClickHotbar(sf::Vector2f mouseScreenPos)
{
    // Get hovered index
    int hoveredIndex = getHotbarHoveredIndex(mouseScreenPos);

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

void InventoryGUI::placeHotbarObject(InventoryData& inventory)
{
    inventory.takeItemAtIndex(selectedHotbarIndex, 1);
}

void InventoryGUI::handleHotbarItemChange()
{
    // Set timer to max
    hotbarItemStringTimer = HOTBAR_ITEM_STRING_OPAQUE_TIME + HOTBAR_ITEM_STRING_FADE_TIME;
}

void InventoryGUI::drawHotbar(sf::RenderTarget& window, sf::Vector2f mouseScreenPos, InventoryData& inventory)
{
    // Get resolution
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();
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

            itemSlot.draw(window, itemCount.first, itemCount.second, false, selected);

            // Set item name string if selected
            if (selected)
            {
                ItemType itemType = itemCount.first;
                selectedItemName = ItemDataLoader::getItemData(itemType).name;
            }
        }
        else
        {
            // Draw blank item box
            itemSlot.draw(window, std::nullopt, std::nullopt, false, selected);
        }
    }

    // Draw item name string
    if (hotbarItemStringTimer > 0)
    {
        // Fade string color if required
        float alpha = std::min(hotbarItemStringTimer / HOTBAR_ITEM_STRING_FADE_TIME, 1.0f) * 255.0f;

        // Draw text
        TextDraw::drawText(window, {
            .text = selectedItemName,
            .position = (hotbarItemSlots[0].getPosition() + sf::Vector2f(0, itemBoxSize)) * intScale,
            .colour = sf::Color(255, 255, 255, alpha),
            .size = 24 * static_cast<unsigned int>(intScale)
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
}

void InventoryGUI::createChestItemSlots(InventoryData* chestData)
{
    if (chestData == nullptr)
        return;
    
    chestItemSlots.clear();

    sf::Vector2f chestItemBoxPosition = sf::Vector2f(itemBoxPadding + (ITEM_BOX_PER_ROW + 2) * itemBoxSize, itemBoxPadding);

    int currentRowIndex = 0;
    for (int itemIndex = 0; itemIndex < chestData->getSize(); itemIndex++)
    {
        ItemSlot chestItemSlot(chestItemBoxPosition, itemBoxSize);

        chestItemSlots.push_back(chestItemSlot);

        // Increment box position
        chestItemBoxPosition.x += itemBoxSize + itemBoxSpacing;

        currentRowIndex++;
        if (currentRowIndex >= CHEST_BOX_PER_ROW)
        {
            // Increment to next row
            currentRowIndex = 0;
            chestItemBoxPosition.x = itemBoxPadding + (ITEM_BOX_PER_ROW + 2) * itemBoxSize;
            chestItemBoxPosition.y += itemBoxSize + itemBoxSpacing;
        }
    }
}

void InventoryGUI::inventoryChestItemQuickTransfer(sf::Vector2f mouseScreenPos, unsigned int amount, InventoryData& inventory, InventoryData& chestData)
{
    InventoryData* hoveredInventory = &inventory;
    InventoryData* destinationInventory = &chestData;

    int itemHovered = getInventoryHoveredIndex(mouseScreenPos);
    if (itemHovered < 0)
    {
        itemHovered = getHoveredChestIndex(mouseScreenPos);
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

void InventoryGUI::pushItemPopup(const ItemCount& itemCount)
{
    if (itemPopups.size() > 0)
    {
        ItemCount& frontItemCount = itemPopups.back().itemCount;

        // Add to item popup if same item is at front of popups
        if (itemCount.first == frontItemCount.first)
        {
            // Reset time and add item count
            itemPopups.back().timeAlive = 0;

            frontItemCount.second += itemCount.second;

            return;
        }
    }

    // Item type popup is not the same as front, so add new popup
    ItemPopup itemPopup;
    itemPopup.itemCount = itemCount;

    itemPopups.push_back(itemPopup);

    if (itemPopups.size() > POPUP_MAX_COUNT)
    {
        itemPopups.erase(itemPopups.begin());
    }
}

void InventoryGUI::drawItemPopups(sf::RenderTarget& window)
{
    if (itemPopups.size() <= 0)
        return;

    sf::RenderTexture popupTexture;
    popupTexture.create(window.getSize().x, window.getSize().y);
    popupTexture.clear(sf::Color(0, 0, 0, 0));

    int intScale = ResolutionHandler::getResolutionIntegerScale();

    sf::Vector2f popupPos(itemBoxPadding * intScale, itemBoxPadding * intScale);

    for (const ItemPopup& itemPopup : itemPopups)
    {
        const ItemData& itemData = ItemDataLoader::getItemData(itemPopup.itemCount.first);

        ItemInfoString infoString;
        infoString.itemCount = itemPopup.itemCount;
        infoString.string = itemData.name;
        infoString.size = 20;

        // Calculate alpha
        int alpha = std::max(std::min((POPUP_LIFETIME - itemPopup.timeAlive) / POPUP_FADE_TIME, 1.0f), 0.0f) * 255;

        sf::Vector2f boxSize = drawInfoBox(popupTexture, popupPos, {infoString}, alpha);
        popupPos.y += boxSize.y + (itemBoxSpacing + 6) * intScale;
    }

    popupTexture.display();

    sf::Sprite popupTextureSprite(popupTexture.getTexture());
    popupTextureSprite.setPosition(sf::Vector2f(0, window.getSize().y - popupPos.y - 9 * intScale));

    window.draw(popupTextureSprite);
}


// -- Misc -- //
bool InventoryGUI::canQuickTransfer(sf::Vector2f mouseScreenPos, bool shiftMode, InventoryData& inventory, InventoryData* chestData)
{
    if (!shiftMode)
        return false;
    
    if (chestData == nullptr)
        return false;
    
    InventoryData* hoveredInventory = &inventory;
    
    int itemHovered = getInventoryHoveredIndex(mouseScreenPos);
    if (itemHovered < 0)
    {
        itemHovered = getHoveredChestIndex(mouseScreenPos);
        hoveredInventory = chestData;
    }

    // Not hovered over inventory / chest
    if (itemHovered < 0)
        return false;
    
    std::optional<ItemCount>& itemSlotData = hoveredInventory->getItemSlotData(itemHovered);

    // No item in hovered slot
    if (!itemSlotData.has_value())
        return false;
    
    return true;
}