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

AnimatedTexture InventoryGUI::binAnimation;
float InventoryGUI::binScale = 1.0f;

// std::array<float, MAX_INVENTORY_SIZE> InventoryGUI::inventoryItemScales;

// std::vector<float> InventoryGUI::chestItemScales;

// std::array<float, InventoryGUI::ITEM_BOX_PER_ROW> InventoryGUI::hotbarItemScales;

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

    // Create recipe item slots
    for (int i = 0; i < availableRecipes.size(); i++)
    {
        ItemSlot recipeItemSlot(sf::Vector2f(xPos, yPos), itemBoxSize);

        recipeItemSlots.push_back(recipeItemSlot);

        // Increment x position
        xPos += itemBoxSize + itemBoxSpacing;
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

void InventoryGUI::handleLeftClick(sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData* chestData)
{
    if (isItemPickedUp)
    {
        putDownItem(mouseScreenPos, inventory, chestData);
    }
    else
    {
        pickUpItem(mouseScreenPos, INVENTORY_STACK_SIZE, inventory, chestData);
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

void InventoryGUI::handleRightClick(sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData* chestData)
{
    pickUpItem(mouseScreenPos, 1, inventory, chestData);
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
    
    const std::optional<ItemCount>* itemSlot = nullptr;

    bool placingInChest = false;

    if (itemIndex >= 0)
    {
        itemSlot = &inventory.getItemSlotData(itemIndex);
    }
    else
    {
        itemSlot = &chestData->getItemSlotData(chestHoveredItemIndex);
        placingInChest = true;
    }

    // If no item, do not pick up
    if (!itemSlot->has_value())
        return;
    
    const ItemCount& itemCount = itemSlot->value();

    // Pick up item
    if (isItemPickedUp)
    {
        // Add to stack already in hand
        if (pickedUpItem == itemCount.first)
        {
            unsigned int amountPickedUp = std::min(amount, INVENTORY_STACK_SIZE - pickedUpItemCount);

            pickedUpItemCount += std::min(itemCount.second, amountPickedUp);

            // Take from inventory / chest
            if (placingInChest)
            {
                // takeItemFromChestAtIndex(chestHoveredItemIndex, amountPickedUp, chestData);
                chestData->takeItemAtIndex(chestHoveredItemIndex, amount);
            }
            else
            {
                inventory.takeItemAtIndex(itemIndex, amountPickedUp);
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
        if (placingInChest)
        {
            // takeItemFromChestAtIndex(chestHoveredItemIndex, amount, chestData);
            chestData->takeItemAtIndex(chestHoveredItemIndex, amount);
        }
        else
        {
            inventory.takeItemAtIndex(itemIndex, amount);
        }
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

    int chestHoveredItemIndex = getHoveredChestIndex(mouseScreenPos);

    // Get item selected at mouse
    int itemIndex = getInventoryHoveredIndex(mouseScreenPos);

    // No valid item selected
    if (itemIndex < 0 && chestHoveredItemIndex < 0)
        return;
    
    const std::optional<ItemCount>* itemSlot = nullptr;

    bool placingInChest = false;

    if (itemIndex >= 0)
    {
        itemSlot = &inventory.getItemSlotData(itemIndex);
    }
    else
    {
        itemSlot = &chestData->getItemSlotData(chestHoveredItemIndex);
        placingInChest = true;
    }

    // If item at selected position, attempt to add to stack
    if (itemSlot->has_value())
    {
        const ItemCount& itemCount = itemSlot->value();

        // Item at selected position is same as in cursor, so add
        // Don't add if stack is full, swap instead
        if (pickedUpItem == itemCount.first && itemCount.second < INVENTORY_STACK_SIZE)
        {
            int amountAddedToStack = std::min(itemCount.second + pickedUpItemCount, INVENTORY_STACK_SIZE) - itemCount.second;

            // Add to stack
            if (placingInChest)
            {
                // addItemToChestAtIndex(chestHoveredItemIndex, pickedUpItem, amountAddedToStack, chestData);
                chestData->addItemAtIndex(chestHoveredItemIndex, pickedUpItem, amountAddedToStack);
            }
            else
            {
                inventory.addItemAtIndex(itemIndex, pickedUpItem, amountAddedToStack);
            }

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
            if (placingInChest)
            {
                // takeItemFromChestAtIndex(chestHoveredItemIndex, itemCount.second, chestData);
                // addItemToChestAtIndex(chestHoveredItemIndex, pickedUpItem, pickedUpItemCount, chestData);
                chestData->takeItemAtIndex(chestHoveredItemIndex, itemCount.second);
                chestData->addItemAtIndex(chestHoveredItemIndex, pickedUpItem, pickedUpItemCount);
            }
            else
            {
                inventory.takeItemAtIndex(itemIndex, itemCount.second);
                inventory.addItemAtIndex(itemIndex, pickedUpItem, pickedUpItemCount);
            }

            // Swap item picked up
            pickedUpItem = toSwap.first;
            pickedUpItemCount = toSwap.second;
        }

        return;
    }

    // No item at selected position, so attempt to put down whole stack
    int amountPutDown = std::min(pickedUpItemCount, static_cast<int>(INVENTORY_STACK_SIZE));

    // Create stack
    if (placingInChest)
    {
        // addItemToChestAtIndex(chestHoveredItemIndex, pickedUpItem, amountPutDown, chestData);
        chestData->addItemAtIndex(chestHoveredItemIndex, pickedUpItem, amountPutDown);
    }
    else
    {
        inventory.addItemAtIndex(itemIndex, pickedUpItem, amountPutDown);
    }

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
    if (isItemPickedUp && (pickedUpItem != recipeData.product || pickedUpItemCount >= INVENTORY_STACK_SIZE))
        return;

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

        // Check items
        bool hasItems = true;
        for (const auto& itemRequired : recipeData.itemRequirements)
        {
            // If player does not have any of the item, cannot craft
            if (inventoryItemCount.count(itemRequired.first) <= 0)
            {
                hasItems = false;
                break;
            }

            // If player has item but not enough, cannot craft
            if (inventoryItemCount[itemRequired.first] < itemRequired.second)
            {
                hasItems = false;
                break;
            }
        }

        if (!hasItems)
            continue;
        
        // Player has items and is near required crafting station, so add to available recipes
        availableRecipes.push_back(recipeIdx);
    }

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

ObjectType InventoryGUI::getHeldObjectType()
{
    if (!isItemPickedUp)
        return -1;
    
    ObjectType placeObjectType = ItemDataLoader::getItemData(pickedUpItem).placesObjectType;

    return placeObjectType;
}

ToolType InventoryGUI::getHeldToolType()
{
    if (!isItemPickedUp)
        return -1;
    
    ToolType toolType = ItemDataLoader::getItemData(pickedUpItem).toolType;

    return toolType;
}

void InventoryGUI::placeHeldObject()
{
    if (!isItemPickedUp)
        return;
    
    pickedUpItemCount--;

    if (pickedUpItemCount <= 0)
    {
        isItemPickedUp = false;
        pickedUpItem = -1;
        pickedUpItemCount = 0;
    }
}

bool InventoryGUI::heldItemPlacesLand()
{
    if (!isItemPickedUp)
        return false;
    
    const ItemData& itemData = ItemDataLoader::getItemData(pickedUpItem);

    return itemData.placesLand;
}

void InventoryGUI::draw(sf::RenderWindow& window, sf::Vector2f mouseScreenPos, InventoryData& inventory, InventoryData* chestData)
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
        int hoveredRecipeIdx = getHoveredRecipe(mouseScreenPos);

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
            bool selected = false;
            if (i == selectedRecipe)
            {
                selected = true;

                int requirementXPos = itemSlot.getPosition().x;
                int requirementYPos = itemSlot.getPosition().y + itemBoxSize;

                for (const auto& itemRequired : recipeData.itemRequirements)
                {
                    // Draw requirement
                    ItemSlot recipeItem(sf::Vector2f(requirementXPos, requirementYPos), itemBoxSize);

                    recipeItem.draw(window, itemRequired.first, itemRequired.second, true);

                    // Increment position
                    requirementYPos += itemBoxSize + itemBoxSpacing;
                }
            }

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
            drawItemInfoBoxRecipe(window, hoveredRecipeIdx, mouseScreenPos);
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
        ItemSlot pickedUpItemSlot(mouseScreenPos - sf::Vector2f(std::round(itemBoxSize / 2.0f), std::round(itemBoxSize / 2.0f)) * intScale, itemBoxSize, false);
        pickedUpItemSlot.draw(window, pickedUpItem, pickedUpItemCount, true);
    }
    else
    {
        // Draw item info box if no item held in cursor and hovering over item
    
        // If an item is hovered over, draw item info box
        if (hoveredItemIndex >= 0)
        {
            drawItemInfoBox(window, hoveredItemIndex, inventory, mouseScreenPos);
        }
        else if (chestData != nullptr)
        {
            // Draw chest hovered item info
            int chestHoveredItemIndex = getHoveredChestIndex(mouseScreenPos);
            if (chestHoveredItemIndex >= 0)
            {
                drawItemInfoBox(window, chestHoveredItemIndex, *chestData, mouseScreenPos);
            }
        }
    }
}

void InventoryGUI::drawItemInfoBox(sf::RenderWindow& window, int itemIndex, InventoryData& inventory, sf::Vector2f mouseScreenPos)
{
    const std::optional<ItemCount>& itemSlot = inventory.getItemSlotData(itemIndex);

    // If no item in slot, do not draw box
    if (!itemSlot.has_value())
        return;
    
    // Get item data
    ItemType itemType = itemSlot.value().first;
    unsigned int itemAmount = itemSlot.value().second;

    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Draw item name
    TextDraw::drawText(window, {
        itemData.name,
        mouseScreenPos - (sf::Vector2f(0, 16)) * intScale,
        {255, 255, 255},
        24 * static_cast<unsigned int>(intScale),
        {0, 0, 0},
        0,
        true,
        true,
        true,
        true
    });
}

void InventoryGUI::drawItemInfoBoxRecipe(sf::RenderWindow& window, int recipeIdx, sf::Vector2f mouseScreenPos)
{
    const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipeIdx];

    const ItemData& itemData = ItemDataLoader::getItemData(recipeData.product);

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Draw item name
    TextDraw::drawText(window, {
        itemData.name,
        mouseScreenPos - (sf::Vector2f(0, 16)) * intScale,
        {255, 255, 255},
        24 * static_cast<unsigned int>(intScale),
        {0, 0, 0},
        0,
        true,
        true,
        true,
        true
    });
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

void InventoryGUI::drawHotbar(sf::RenderWindow& window, sf::Vector2f mouseScreenPos, InventoryData& inventory)
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

// void InventoryGUI::addItemToChestAtIndex(int index, ItemType item, int amount, InventoryData* chestData)
// {
//     if (chestData == nullptr)
//         return;
    
//     if (index >= MAX_INVENTORY_SIZE)
//         return;
    
//     std::optional<ItemCount>& itemSlot = chestData->getItemSlotData(index);

//     // Attempt to add to stack
//     if (itemSlot.has_value())
//     {
//         ItemCount& itemCount = itemSlot.value();

//         // Item to add is same as item at index, so add
//         if (item == itemCount.first)
//         {
//             itemCount.second = std::min(itemCount.second + amount, INVENTORY_STACK_SIZE);
//         }

//         return;
//     }

//     // Create new stack
//     ItemCount itemCount;
//     itemCount.first = item;
//     itemCount.second = std::min(amount, static_cast<int>(INVENTORY_STACK_SIZE));

//     itemSlot = itemCount;
// }

// void InventoryGUI::takeItemFromChestAtIndex(int index, int amount, InventoryData* chestData)
// {
//     if (chestData == nullptr)
//         return;
    
//     if (index >= MAX_INVENTORY_SIZE)
//         return;
    
//     std::optional<ItemCount>& itemSlot = chestData->getItemSlotData(index);
//     if (!itemSlot.has_value())
//         return;
    
//     ItemCount& itemCount = itemSlot.value();

//     if (amount >= itemCount.second)
//     {
//         itemSlot = std::nullopt;
//     }
//     else
//     {
//         itemCount.second -= amount;
//     }
// }