#include "GUI/InventoryGUI.hpp"

sf::Vector2f InventoryGUI::screenPos;

int InventoryGUI::itemBoxSize = 75;
int InventoryGUI::itemBoxSpacing = 10;
int InventoryGUI::itemBoxPadding = 10;
int InventoryGUI::itemBoxPerRow = 8;

bool InventoryGUI::isItemPickedUp = false;
ItemType InventoryGUI::pickedUpItem;
int InventoryGUI::pickedUpItemCount;

std::unordered_map<std::string, int> InventoryGUI::previous_nearbyCraftingStationLevels;
std::unordered_map<ItemType, unsigned int> InventoryGUI::previous_inventoryItemCount;
std::vector<int> InventoryGUI::availableRecipes;

int InventoryGUI::selectedRecipe;

void InventoryGUI::handleLeftClick(sf::Vector2f mouseScreenPos)
{
    if (isItemPickedUp)
    {
        putDownItem(mouseScreenPos);
    }
    else
    {
        pickUpItem(mouseScreenPos);
    }
}

void InventoryGUI::handleRightClick(sf::Vector2f mouseScreenPos)
{
    pickUpItem(mouseScreenPos, 1);
}

bool InventoryGUI::handleScroll(sf::Vector2f mouseScreenPos, int direction)
{
    if (!isMouseOverUI(mouseScreenPos))
        return false;
    
    selectedRecipe = ((selectedRecipe + direction) % availableRecipes.size() + availableRecipes.size()) % availableRecipes.size();

    return true;
}

void InventoryGUI::pickUpItem(sf::Vector2f mouseScreenPos, unsigned int amount)
{
    // Get item selected at mouse
    int itemIndex = getInventorySelectedIndex(mouseScreenPos);

    // No valid item selected
    if (itemIndex < 0)
        return;
    
    const std::optional<ItemCount>& itemSlot = Inventory::getData()[itemIndex];

    // If no item, do not pick up
    if (!itemSlot.has_value())
        return;
    
    const ItemCount& itemCount = itemSlot.value();

    // Pick up item
    if (isItemPickedUp)
    {
        // Add to stack already in hand
        if (pickedUpItem == itemCount.first)
        {
            unsigned int amountPickedUp = std::min(amount, INVENTORY_STACK_SIZE - pickedUpItemCount);

            pickedUpItemCount += std::min(itemCount.second, amountPickedUp);

            // Take from inventory
            Inventory::takeItemAtIndex(itemIndex, amountPickedUp);
        }
    }
    else
    {
        // Create new stack in hand
        isItemPickedUp = true;
        pickedUpItem = itemCount.first;
        pickedUpItemCount = std::min(itemCount.second, amount);

        // Take from inventory
        Inventory::takeItemAtIndex(itemIndex, amount);
    }
}

void InventoryGUI::putDownItem(sf::Vector2f mouseScreenPos)
{
    if (!isItemPickedUp)
        return;

    // If bin selected, delete item and return
    if (isBinSelected(mouseScreenPos))
    {
        isItemPickedUp = false;
        pickedUpItem = 0;
        pickedUpItemCount = 0;
        return;
    }

    // Get item selected at mouse
    int itemIndex = getInventorySelectedIndex(mouseScreenPos);

    // No valid item selected
    if (itemIndex < 0)
        return;
    
    const std::optional<ItemCount>& itemSlot = Inventory::getData()[itemIndex];

    // If item at selected position, attempt to add to stack
    if (itemSlot.has_value())
    {
        const ItemCount& itemCount = itemSlot.value();

        // Item at selected position is same as in cursor, so add
        // Don't add if stack is full, swap instead
        if (pickedUpItem == itemCount.first && itemCount.second < INVENTORY_STACK_SIZE)
        {
            int amountAddedToStack = std::min(itemCount.second + pickedUpItemCount, INVENTORY_STACK_SIZE) - itemCount.second;

            // Add to stack
            Inventory::addItemAtIndex(itemIndex, pickedUpItem, amountAddedToStack);

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

            // Swap in inventory
            Inventory::takeItemAtIndex(itemIndex, itemCount.second);
            Inventory::addItemAtIndex(itemIndex, pickedUpItem, pickedUpItemCount);

            // Swap item picked up
            pickedUpItem = toSwap.first;
            pickedUpItemCount = toSwap.second;
        }

        return;
    }

    // No item at selected position, so attempt to put down whole stack
    int amountPutDown = std::min(pickedUpItemCount, static_cast<int>(INVENTORY_STACK_SIZE));

    // Create stack
    Inventory::addItemAtIndex(itemIndex, pickedUpItem, amountPutDown);

    // Take from cursor
    pickedUpItemCount -= amountPutDown;

    if (pickedUpItemCount <= 0)
        isItemPickedUp = false;
}

int InventoryGUI::getInventorySelectedIndex(sf::Vector2f mouseScreenPos)
{
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();
    int intScale = ResolutionHandler::getResolutionIntegerScale();

    // Mimic GUI and create collision rects to test if item is being picked up
    sf::Vector2f itemBoxPosition = sf::Vector2f(itemBoxPadding, itemBoxPadding) * static_cast<float>(intScale);

    int currentRowIndex = 0;

    for (int itemIndex = 0; itemIndex < Inventory::getData().size(); itemIndex++)
    {
        CollisionRect itemPickUpRect;
        
        itemPickUpRect.width = itemBoxSize * intScale;
        itemPickUpRect.height = itemBoxSize * intScale;
        itemPickUpRect.x = itemBoxPosition.x;
        itemPickUpRect.y = itemBoxPosition.y;

        // Test if in item box area
        if (itemPickUpRect.isPointInRect(mouseScreenPos.x, mouseScreenPos.y))
        {
            return itemIndex;
        }

        // Increment box position
        itemBoxPosition.x += (itemBoxSize + itemBoxSpacing) * intScale;

        currentRowIndex++;
        if (currentRowIndex >= itemBoxPerRow)
        {
            // Increment to next row
            currentRowIndex = 0;
            itemBoxPosition.x = itemBoxPadding * intScale;
            itemBoxPosition.y += (itemBoxSize + itemBoxSpacing) * intScale;
        }
    }

    // Default case
    return -1;
}

bool InventoryGUI::isBinSelected(sf::Vector2f mouseScreenPos)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
     
    sf::Vector2f binPos = sf::Vector2f(itemBoxPadding, itemBoxPadding) * intScale + sf::Vector2f(itemBoxSize + itemBoxSpacing, 0) * static_cast<float>(itemBoxPerRow) * intScale;

    CollisionRect binCollisionRect;
    binCollisionRect.x = binPos.x;
    binCollisionRect.y = binPos.y;
    binCollisionRect.width = 16.0f * 3 * intScale;
    binCollisionRect.height = 16.0f * 3 * intScale;

    return binCollisionRect.isPointInRect(mouseScreenPos.x, mouseScreenPos.y);
}

void InventoryGUI::handleClose()
{
    // Handle item still picked up when inventory is closed
    if (isItemPickedUp)
    {
        isItemPickedUp = false;

        Inventory::addItem(pickedUpItem, pickedUpItemCount);
    }
}

bool InventoryGUI::isMouseOverUI(sf::Vector2f mouseScreenPos)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    CollisionRect uiMask;

    uiMask.x = itemBoxPadding * intScale;
    uiMask.y = itemBoxPadding * intScale;
    uiMask.width = ((itemBoxSize + itemBoxSpacing) * itemBoxPerRow - itemBoxSpacing) * intScale;
    uiMask.height = ((itemBoxSize + itemBoxSpacing) * std::round(Inventory::getData().size() / itemBoxPerRow) - itemBoxSpacing) * intScale;

    return uiMask.isPointInRect(mouseScreenPos.x, mouseScreenPos.y) || isBinSelected(mouseScreenPos);
}

void InventoryGUI::updateAvailableRecipes(std::unordered_map<std::string, int> nearbyCraftingStationLevels)
{
    std::unordered_map<ItemType, unsigned int> inventoryItemCount = Inventory::getTotalItemCount();

    // If crafting stations and items have not changed, do not update recipes
    if (nearbyCraftingStationLevels == previous_nearbyCraftingStationLevels && inventoryItemCount == previous_inventoryItemCount)
        return;
    
    previous_nearbyCraftingStationLevels = nearbyCraftingStationLevels;
    previous_inventoryItemCount = inventoryItemCount;

    // Reset available recipes
    availableRecipes.clear();

    // Iterate over all recipes and determine if available to player
    for (int recipeIdx = 0; recipeIdx <= RecipeDataLoader::getRecipeData().size(); recipeIdx++)
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

    if (availableRecipes.size() > 0)
    {
        selectedRecipe = (selectedRecipe % availableRecipes.size() + availableRecipes.size()) % availableRecipes.size();
    }
    else
    {
        selectedRecipe = 0;
    }
}

void InventoryGUI::draw(sf::RenderWindow& window, sf::Vector2f mouseScreenPos)
{
    // Get resolution
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();


    // Draw items
    sf::Vector2f itemBoxPosition = sf::Vector2f(itemBoxPadding, itemBoxPadding) * intScale;

    int currentRowIndex = 0;

    for (const std::optional<ItemCount>& itemSlot : Inventory::getData())
    {
        if (itemSlot.has_value())
        {
            const ItemCount& itemCount = itemSlot.value();

            drawItemBox(window, itemBoxPosition, itemCount.first, itemCount.second);
        }
        else
        {
            // Draw blank item box
            drawItemBox(window, itemBoxPosition);
        }

        itemBoxPosition.x += (itemBoxSize + itemBoxSpacing) * intScale;

        currentRowIndex++;
        if (currentRowIndex >= itemBoxPerRow)
        {
            // Increment to next row
            currentRowIndex = 0;
            itemBoxPosition.x = itemBoxPadding * intScale;
            itemBoxPosition.y += (itemBoxSize + itemBoxSpacing) * intScale;
        }
    }

    // Draw bin
    TextureManager::drawSubTexture(window, {
        TextureType::UI,
        sf::Vector2f(itemBoxPadding, itemBoxPadding) * intScale + sf::Vector2f(itemBoxSize + itemBoxSpacing, 0) * static_cast<float>(itemBoxPerRow) * intScale,
        0,
        {3 * intScale, 3 * intScale},
    }, sf::IntRect(64, 16, 16, 16));


    // Draw Available recipes
    if (availableRecipes.size() > 0)
    {
        // Calculate starting x position of recipes
        int xPos = itemBoxPadding * intScale;

        // Calculate y position
        int yPos = itemBoxPadding + ((itemBoxSize + itemBoxSpacing) * std::round(Inventory::getData().size() / itemBoxPerRow) - itemBoxSpacing) * intScale;
        yPos += itemBoxSize * intScale;

        // Store recipe index hovered over for drawing
        int hoveredRecipeIdx = -1;

        // Draw recipes
        for (int i = selectedRecipe; i < availableRecipes.size() + selectedRecipe; i++)
        {
            // Get recipe index
            int recipeIdx = availableRecipes[i % availableRecipes.size()];

            // Get recipe data
            const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipeIdx];

            // If recipe is selected, draw requirements
            bool selected = false;
            if (i == selectedRecipe)
            {
                selected = true;

                int requirementXPos = itemBoxPadding * intScale;
                int requirementYPos = yPos + itemBoxSize * intScale;

                for (const auto& itemRequired : recipeData.itemRequirements)
                {
                    // Draw requirement
                    drawItemBox(window, sf::Vector2f(requirementXPos, requirementYPos), itemRequired.first, itemRequired.second, true);

                    // Increment position
                    requirementYPos += (itemBoxSize + itemBoxSpacing) * intScale;
                }
            }

            // Draw item box for product
            drawItemBox(window, sf::Vector2f(xPos, yPos), recipeData.product, recipeData.productAmount, false, selected);

            // Test whether mouse is over - if so, store recipe index for drawing info later
            CollisionRect recipeRect(xPos, yPos, itemBoxSize * intScale, itemBoxSize * intScale);
            if (recipeRect.isPointInRect(mouseScreenPos.x, mouseScreenPos.y))
            {
                hoveredRecipeIdx = recipeIdx;
            }

            // Increment x position
            xPos += (itemBoxSize + itemBoxSpacing) * intScale;
        }

        // Draw hammer icon
        TextureManager::drawSubTexture(window, {
            TextureType::UI,
            sf::Vector2f(xPos, yPos),
            0,
            {3 * intScale, 3 * intScale},
        }, sf::IntRect(80, 16, 16, 16));

        // Draw info of recipe hovered over (if any)
        if (hoveredRecipeIdx >= 0)
        {
            drawItemInfoBoxRecipe(window, hoveredRecipeIdx, mouseScreenPos);
        }
    }

    // Draw picked up item at cursor
    if (isItemPickedUp)
    {
        drawItemBox(window, mouseScreenPos - sf::Vector2f(std::round(itemBoxSize / 2.0f), std::round(itemBoxSize / 2.0f)) * intScale, pickedUpItem, pickedUpItemCount, true);
    }
    else
    {
        // Draw item info box if no item held in cursor and hovering over item
        int selectedItemIndex = getInventorySelectedIndex(mouseScreenPos);

        // If an item is hovered over, draw item info box
        if (selectedItemIndex >= 0)
        {
            drawItemInfoBoxInventory(window, selectedItemIndex, mouseScreenPos);
        }
    }
}

void InventoryGUI::drawItemInfoBoxInventory(sf::RenderWindow& window, int itemIndex, sf::Vector2f mouseScreenPos)
{
    const std::optional<ItemCount>& itemSlot = Inventory::getData()[itemIndex];

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

void InventoryGUI::drawItemBox(sf::RenderWindow& window,
                               sf::Vector2f position,
                               std::optional<ItemType> itemType,
                               std::optional<int> itemAmount,
                               bool hiddenBackground,
                               bool selectHighlight)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Draw background
    if (!hiddenBackground)
    {
        sf::IntRect boxRect(16, 16, 25, 25);
        if (selectHighlight)
        {
            boxRect = sf::IntRect(16, 48, 25, 25);
        }

        TextureManager::drawSubTexture(window, {
            TextureType::UI,
            position,
            0,
            {3 * intScale, 3 * intScale},
        }, boxRect);
    }

    // Draw item
    if (itemType.has_value())
    {
        TextureManager::drawSubTexture(window, {
            TextureType::Items,
            position + (sf::Vector2f(std::round(itemBoxSize / 2.0f), std::round(itemBoxSize / 2.0f))) * intScale,
            0,
            {3 * intScale, 3 * intScale},
            {0.5, 0.5}
        }, ItemDataLoader::getItemData(itemType.value()).textureRect);
    }

    // Draw item count (if > 1)
    if (itemAmount.has_value())
    {
        if (itemAmount.value() > 1)
        {
            TextDraw::drawText(window, {
                std::to_string(itemAmount.value()),
                position + (sf::Vector2f(std::round(itemBoxSize / 4.0f) * 3.0f, std::round(itemBoxSize / 4.0f) * 3.0f)) * intScale,
                {255, 255, 255},
                24 * static_cast<unsigned int>(intScale),
                {0, 0, 0},
                0,
                true,
                true});
        }
    }
}