#include "GUI/InventoryGUI.hpp"

sf::Vector2f InventoryGUI::screenPos;

int InventoryGUI::itemBoxSize = 75;
int InventoryGUI::itemBoxSpacing = 10;
int InventoryGUI::itemBoxPadding = 10;

bool InventoryGUI::isItemPickedUp = false;
ItemType InventoryGUI::pickedUpItem;
int InventoryGUI::pickedUpItemCount;

std::unordered_map<std::string, int> InventoryGUI::previous_nearbyCraftingStationLevels;
std::unordered_map<ItemType, unsigned int> InventoryGUI::previous_inventoryItemCount;
std::vector<int> InventoryGUI::availableRecipes;

int InventoryGUI::selectedRecipe = 0;

int InventoryGUI::selectedHotbarIndex = 0;

AnimatedTexture InventoryGUI::binAnimation;
float InventoryGUI::binScale = 1.0f;

std::array<float, MAX_INVENTORY_SIZE> InventoryGUI::inventoryItemScales;

std::array<float, InventoryGUI::ITEM_BOX_PER_ROW> InventoryGUI::hotbarItemScales;

float InventoryGUI::hotbarItemStringTimer = 0.0f;

void InventoryGUI::initialise()
{
    binAnimation.create(4, 16, 20, 96, 12, 0.04f, false);

    inventoryItemScales.fill(1.0f);

    hotbarItemScales.fill(1.0f);
}

void InventoryGUI::updateAnimations(sf::Vector2f mouseScreenPos, float dt)
{
    // Update bin animation
    int binAnimationUpdateDirection = 0;
    if (isBinSelected(mouseScreenPos) && isItemPickedUp)
    {
        binAnimationUpdateDirection = 1;
        binScale = Helper::lerp(binScale, BIN_HOVERED_SCALE, ITEM_HOVERED_SCALE_LERP_WEIGHT * dt);
    }
    else
    {
        binAnimationUpdateDirection = -1;
        binScale = Helper::lerp(binScale, 1.0f, ITEM_HOVERED_SCALE_LERP_WEIGHT * dt);
    }

    binAnimation.update(dt, binAnimationUpdateDirection);

    // Update item scales
    int hoveredItem = getInventoryHoveredIndex(mouseScreenPos);

    for (int i = 0; i < inventoryItemScales.size(); i++)
    {
        float& itemScale = inventoryItemScales[i];

        if (i == hoveredItem)
        {
            // Scale up
            itemScale = Helper::lerp(itemScale, ITEM_HOVERED_SCALE, ITEM_HOVERED_SCALE_LERP_WEIGHT * dt);
            continue;
        }

        // Scale down to 1.0f
        itemScale = Helper::lerp(itemScale, 1.0f, ITEM_HOVERED_SCALE_LERP_WEIGHT * dt);
    }
}

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

    int recipeClicked = getHoveredRecipe(mouseScreenPos);
    if (recipeClicked >= 0)
    {
        // If clicked on recipe is selected, attempt to craft item
        if (recipeClicked == selectedRecipe)
        {
            craftSelectedRecipe();
        }
        else
        {
            // Change selected recipe to recipe clicked on
            selectedRecipe = recipeClicked;
        }
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
    
    if (availableRecipes.size() > 0)
    {
        selectedRecipe = ((selectedRecipe + direction) % availableRecipes.size() + availableRecipes.size()) % availableRecipes.size();
    }

    return true;
}

void InventoryGUI::pickUpItem(sf::Vector2f mouseScreenPos, unsigned int amount)
{
    // Get item selected at mouse
    int itemIndex = getInventoryHoveredIndex(mouseScreenPos);

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
        pickedUpItem = -1;
        pickedUpItemCount = 0;
        return;
    }

    // Get item selected at mouse
    int itemIndex = getInventoryHoveredIndex(mouseScreenPos);

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

int InventoryGUI::getInventoryHoveredIndex(sf::Vector2f mouseScreenPos, bool onlyHotbar)
{
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();
    int intScale = ResolutionHandler::getResolutionIntegerScale();

    // Mimic GUI and create collision rects to test if item is being picked up
    sf::Vector2f itemBoxPosition = sf::Vector2f(itemBoxPadding, itemBoxPadding) * static_cast<float>(intScale);

    int itemBoxCount = Inventory::getData().size();
    if (onlyHotbar)
    {
        itemBoxCount = ITEM_BOX_PER_ROW;
    }

    int currentRowIndex = 0;

    for (int itemIndex = 0; itemIndex < itemBoxCount; itemIndex++)
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
        if (currentRowIndex >= ITEM_BOX_PER_ROW)
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

int InventoryGUI::getHoveredRecipe(sf::Vector2f mouseScreenPos)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Mimic recipe GUI
    if (availableRecipes.size() > 0)
    {
        // Calculate starting x position of recipes
        int xPos = itemBoxPadding * intScale;

        // Calculate y position
        int yPos = itemBoxPadding + ((itemBoxSize + itemBoxSpacing) * std::round(Inventory::getData().size() / ITEM_BOX_PER_ROW) - itemBoxSpacing) * intScale;
        yPos += itemBoxSize * intScale;

        // Iterate over recipes on screen
        for (int i = selectedRecipe; i < availableRecipes.size() + selectedRecipe; i++)
        {
            // Test if clicked on
            CollisionRect recipeRect(xPos, yPos, itemBoxSize * intScale, itemBoxSize * intScale);

            if (recipeRect.isPointInRect(mouseScreenPos.x, mouseScreenPos.y))
            {
                return i % availableRecipes.size();
            }

            // Increment x position
            xPos += (itemBoxSize + itemBoxSpacing) * intScale;
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
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    CollisionRect uiMask;

    uiMask.x = itemBoxPadding * intScale;
    uiMask.y = itemBoxPadding * intScale;
    uiMask.width = ((itemBoxSize + itemBoxSpacing) * ITEM_BOX_PER_ROW - itemBoxSpacing) * intScale;
    uiMask.height = ((itemBoxSize + itemBoxSpacing) * std::round(Inventory::getData().size() / ITEM_BOX_PER_ROW) - itemBoxSpacing) * intScale;

    return uiMask.isPointInRect(mouseScreenPos.x, mouseScreenPos.y);
}

bool InventoryGUI::isCraftingSelected(sf::Vector2f mouseScreenPos)
{
    if (availableRecipes.size() <= 0)
        return false;

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    CollisionRect uiMask;

    uiMask.x = itemBoxPadding * intScale;
    uiMask.y = itemBoxPadding * intScale + ((itemBoxSize + itemBoxSpacing) * std::round(Inventory::getData().size() / ITEM_BOX_PER_ROW) - itemBoxSpacing) * intScale;
    uiMask.width = ((itemBoxSize + itemBoxSpacing) * (availableRecipes.size() + 1) - itemBoxSpacing) * intScale;
    uiMask.height = (itemBoxSize + itemBoxSpacing) * 3 * intScale;

    return uiMask.isPointInRect(mouseScreenPos.x, mouseScreenPos.y);
}

void InventoryGUI::craftSelectedRecipe()
{
    // Get recipe data
    int recipeIdx = availableRecipes[selectedRecipe];
    const RecipeData& recipeData = RecipeDataLoader::getRecipeData()[recipeIdx];

    // If item in hand and is not item to be crafted, do not craft recipe (as won't be able to pick up crafted item)
    if (isItemPickedUp && (pickedUpItem != recipeData.product || pickedUpItemCount >= INVENTORY_STACK_SIZE))
        return;

    // Get inventory to check if has items (should have items as recipe is in available recipes, check just in case)
    std::unordered_map<ItemType, unsigned int> inventoryItemCount = Inventory::getTotalItemCount();

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
        Inventory::takeItem(itemRequired.first, itemRequired.second);
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
    updateAvailableRecipes(previous_nearbyCraftingStationLevels);
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
    return isInventorySelected(mouseScreenPos) || isCraftingSelected(mouseScreenPos) || isBinSelected(mouseScreenPos);
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

    if (availableRecipes.size() > 0)
    {
        selectedRecipe = (selectedRecipe % availableRecipes.size() + availableRecipes.size()) % availableRecipes.size();
    }
    else
    {
        selectedRecipe = 0;
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

void InventoryGUI::draw(sf::RenderWindow& window, sf::Vector2f mouseScreenPos)
{
    // Get resolution
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Get currently hovered over item
    int hoveredItemIndex = getInventoryHoveredIndex(mouseScreenPos);

    // Get currently hovered recipe
    int hoveredRecipeIndex = getHoveredRecipe(mouseScreenPos);

    // Draw items
    sf::Vector2f itemBoxPosition = sf::Vector2f(itemBoxPadding, itemBoxPadding) * intScale;

    int currentRowIndex = 0;

    for (int i = 0; i < Inventory::getData().size(); i++)
    {
        const std::optional<ItemCount>& itemSlot = Inventory::getData()[i];

        if (itemSlot.has_value())
        {
            const ItemCount& itemCount = itemSlot.value();

            // Scale item up slightly if hovered over
            float itemScaleMult = inventoryItemScales[i];

            drawItemBox(window, itemBoxPosition, itemCount.first, itemCount.second, false, false, itemScaleMult);
        }
        else
        {
            // Draw blank item box
            drawItemBox(window, itemBoxPosition);
        }

        itemBoxPosition.x += (itemBoxSize + itemBoxSpacing) * intScale;

        currentRowIndex++;
        if (currentRowIndex >= ITEM_BOX_PER_ROW)
        {
            // Increment to next row
            currentRowIndex = 0;
            itemBoxPosition.x = itemBoxPadding * intScale;
            itemBoxPosition.y += (itemBoxSize + itemBoxSpacing) * intScale;
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
    if (availableRecipes.size() > 0)
    {
        // Calculate starting x position of recipes
        int xPos = itemBoxPadding * intScale;

        // Calculate y position
        int yPos = itemBoxPadding + ((itemBoxSize + itemBoxSpacing) * std::round(Inventory::getData().size() / ITEM_BOX_PER_ROW) - itemBoxSpacing) * intScale;
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
        if (hoveredRecipeIdx >= 0 && !isItemPickedUp)
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
    
        // If an item is hovered over, draw item info box
        if (hoveredItemIndex >= 0)
        {
            drawItemInfoBoxInventory(window, hoveredItemIndex, mouseScreenPos);
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
                               bool selectHighlight,
                               float itemScaleMult)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Draw background
    if (!hiddenBackground)
    {
        sf::IntRect boxRect(16, 16, 25, 25);
        sf::Vector2f origin(0, 0);
        if (selectHighlight)
        {
            boxRect = sf::IntRect(16, 48, 27, 27);
            origin = sf::Vector2f(1 / 27.0f, 1 / 27.0f);
        }

        TextureManager::drawSubTexture(window, {
            TextureType::UI,
            position,
            0,
            {3 * intScale, 3 * intScale},
            origin
        }, boxRect);
    }

    // Draw item
    if (itemType.has_value())
    {
        const ItemData& itemData = ItemDataLoader::getItemData(itemType.value());

        // Draw object if item places object
        if (itemData.placesObjectType >= 0)
        {
            const ObjectData& objectData = ObjectDataLoader::getObjectData(itemData.placesObjectType);

            float objectScale = std::max(4 - std::max(objectData.textureRects[0].width / 16.0f, objectData.textureRects[0].height / 16.0f), 1.0f) * itemScaleMult;

            // Draw object
            TextureManager::drawSubTexture(window, {
                TextureType::Objects,
                position + (sf::Vector2f(std::round(itemBoxSize / 2.0f), std::round(itemBoxSize / 2.0f))) * intScale,
                0,
                {objectScale * intScale, objectScale * intScale},
                {0.5, 0.5}
            }, objectData.textureRects[0]);
        }
        else if (itemData.toolType >= 0)
        {
            // Draw tool
            const ToolData& toolData = ToolDataLoader::getToolData(itemData.toolType);

            float objectScale = std::max(4 - std::max(toolData.textureRect.width / 16.0f, toolData.textureRect.height / 16.0f), 1.0f) * itemScaleMult;

            // Draw object
            TextureManager::drawSubTexture(window, {
                TextureType::Tools,
                position + (sf::Vector2f(std::round(itemBoxSize / 2.0f), std::round(itemBoxSize / 2.0f))) * intScale,
                0,
                {objectScale * intScale, objectScale * intScale},
                {0.5, 0.5}
            }, toolData.textureRect);
        }
        else
        {
            // Draw as normal
            TextureManager::drawSubTexture(window, {
                TextureType::Items,
                position + (sf::Vector2f(std::round(itemBoxSize / 2.0f), std::round(itemBoxSize / 2.0f))) * intScale,
                0,
                {3 * itemScaleMult * intScale, 3 * itemScaleMult * intScale},
                {0.5, 0.5}
            }, itemData.textureRect);
        }
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


// -- Hotbar -- //

void InventoryGUI::updateAnimationsHotbar(float dt, sf::Vector2f mouseScreenPos)
{
    int hoveredItem = getInventoryHoveredIndex(mouseScreenPos, true);

    for (int i = 0; i < hotbarItemScales.size(); i++)
    {
        float& itemScale = hotbarItemScales[i];

        if (i == selectedHotbarIndex ||i == hoveredItem)
        {
            // Scale up
            itemScale = Helper::lerp(itemScale, HOTBAR_SELECTED_SCALE, ITEM_HOVERED_SCALE_LERP_WEIGHT * dt);
            continue;
        }

        // Scale down to 1.0f
        itemScale = Helper::lerp(itemScale, 1.0f, ITEM_HOVERED_SCALE_LERP_WEIGHT * dt);
    }

    // Update item string timer
    hotbarItemStringTimer = std::max(hotbarItemStringTimer - dt, 0.0f);
}

bool InventoryGUI::handleLeftClickHotbar(sf::Vector2f mouseScreenPos)
{
    // Get hovered index
    int hoveredIndex = getInventoryHoveredIndex(mouseScreenPos, true);

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

ObjectType InventoryGUI::getHotbarSelectedObject()
{
    // Get item
    const std::optional<ItemCount>& itemSlot = Inventory::getData()[selectedHotbarIndex];

    if (!itemSlot.has_value())
        return -1;
    
    ItemType itemType = itemSlot.value().first;

    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    return itemData.placesObjectType;
}

ToolType InventoryGUI::getHotbarSelectedTool()
{
    // Get item
    const std::optional<ItemCount>& itemSlot = Inventory::getData()[selectedHotbarIndex];

    if (!itemSlot.has_value())
        return -1;
    
    ItemType itemType = itemSlot.value().first;

    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    return itemData.toolType;
}

bool InventoryGUI::hotbarItemPlacesLand()
{
    // Get item
    const std::optional<ItemCount>& itemSlot = Inventory::getData()[selectedHotbarIndex];

    if (!itemSlot.has_value())
        return false;
    
    ItemType itemType = itemSlot.value().first;

    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    return itemData.placesLand;
}

void InventoryGUI::placeHotbarObject()
{
    Inventory::takeItemAtIndex(selectedHotbarIndex, 1);
}

void InventoryGUI::handleHotbarItemChange()
{
    // Set timer to max
    hotbarItemStringTimer = HOTBAR_ITEM_STRING_OPAQUE_TIME + HOTBAR_ITEM_STRING_FADE_TIME;
}

void InventoryGUI::drawHotbar(sf::RenderWindow& window, sf::Vector2f mouseScreenPos)
{
    // Get resolution
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    sf::Vector2f itemBoxPosition = sf::Vector2f(itemBoxPadding, itemBoxPadding) * intScale;

    std::string selectedItemName = "";

    // Draw first row of inventory
    for (int i = 0; i < ITEM_BOX_PER_ROW; i++)
    {
        const std::optional<ItemCount>& itemSlot = Inventory::getData()[i];

        bool selected = false;
        if (i == selectedHotbarIndex)
        {
            selected = true;
        }

        if (itemSlot.has_value())
        {
            const ItemCount& itemCount = itemSlot.value();

            // Scale item up slightly if hovered over
            float itemScaleMult = hotbarItemScales[i];

            drawItemBox(window, itemBoxPosition, itemCount.first, itemCount.second, false, selected, itemScaleMult);

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
            drawItemBox(window, itemBoxPosition, std::nullopt, std::nullopt, false, selected);
        }

        itemBoxPosition.x += (itemBoxSize + itemBoxSpacing) * intScale;
    }

    // Draw item name string
    if (hotbarItemStringTimer > 0)
    {
        // Fade string color if required
        float alpha = std::min(hotbarItemStringTimer / HOTBAR_ITEM_STRING_FADE_TIME, 1.0f) * 255.0f;

        // Draw text
        TextDraw::drawText(window, {
            .text = selectedItemName,
            .position = sf::Vector2f(itemBoxPadding, itemBoxPadding + itemBoxSize) * intScale,
            .colour = sf::Color(255, 255, 255, alpha),
            .size = 24 * static_cast<unsigned int>(intScale)
        });
    }
}