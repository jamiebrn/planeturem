#include "GUI/InventoryGUI.hpp"

sf::Vector2f InventoryGUI::screenPos;

int InventoryGUI::itemBoxSize = 75;
int InventoryGUI::itemBoxSpacing = 10;
int InventoryGUI::itemBoxPadding = 10;
int InventoryGUI::itemBoxPerRow = 8;

bool InventoryGUI::isItemPickedUp = false;
ItemType InventoryGUI::pickedUpItem;
int InventoryGUI::pickedUpItemCount;

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

    uiMask.x = itemBoxPadding;
    uiMask.y = itemBoxPadding;
    uiMask.width = ((itemBoxSize + itemBoxSpacing) * itemBoxPerRow - itemBoxSpacing) * intScale;
    uiMask.height = ((itemBoxSize + itemBoxSpacing) * std::round(Inventory::getData().size() / itemBoxPerRow) - itemBoxSpacing) * intScale;

    return uiMask.isPointInRect(mouseScreenPos.x, mouseScreenPos.y) || isBinSelected(mouseScreenPos);
}

void InventoryGUI::draw(sf::RenderWindow& window, sf::Vector2f mouseScreenPos)
{
    // Get resolution
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    // Draw background
    // sf::RectangleShape background({800 * static_cast<float>(intScale), 400 * static_cast<float>(intScale)});

    // background.setOrigin({400, 200});
    // background.setPosition({10 * static_cast<float>(intScale), 10 * static_cast<float>(intScale)});
    // background.setFillColor({40, 40, 40, 130});

    // window.draw(background);

    // Draw items

    sf::Vector2f itemBoxPosition = sf::Vector2f(itemBoxPadding, itemBoxPadding) * intScale;

    int currentRowIndex = 0;

    for (const std::optional<ItemCount>& itemSlot : Inventory::getData())
    {
        // sf::RectangleShape itemBackground({75 * intScale, 75 * intScale});

        // itemBackground.setOrigin({40, 40});
        // itemBackground.setPosition(itemBoxPosition);
        // itemBackground.setFillColor({40, 40, 40, 140});

        // window.draw(itemBackground);

        TextureManager::drawSubTexture(window, {
                TextureType::UI,
                itemBoxPosition,
                0,
                {3 * intScale, 3 * intScale},
            }, sf::IntRect(16, 16, 25, 25));

        if (itemSlot.has_value())
        {
            const ItemCount& itemCount = itemSlot.value();

            TextureManager::drawSubTexture(window, {
                TextureType::Items,
                itemBoxPosition + (sf::Vector2f(std::round(itemBoxSize / 2.0f), std::round(itemBoxSize / 2.0f))) * intScale,
                0,
                {3 * intScale, 3 * intScale},
                {0.5, 0.5}
            }, ItemDataLoader::getItemData(itemCount.first).textureRect);

            TextDraw::drawText(window, {
                std::to_string(itemCount.second),
                itemBoxPosition + (sf::Vector2f(std::round(itemBoxSize / 4.0f) * 3.0f, std::round(itemBoxSize / 4.0f) * 3.0f)) * intScale,
                {255, 255, 255},
                24 * static_cast<unsigned int>(intScale),
                {0, 0, 0},
                0,
                true,
                true});
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

    // Draw picked up item at cursor
    if (isItemPickedUp)
    {
        TextureManager::drawSubTexture(window, {
            TextureType::Items,
            mouseScreenPos,
            0,
            {3 * intScale, 3 * intScale},
            {0.5, 0.5}
        }, ItemDataLoader::getItemData(pickedUpItem).textureRect);

        TextDraw::drawText(window, {
            std::to_string(pickedUpItemCount),
            mouseScreenPos + (sf::Vector2f(std::round(itemBoxSize / 4.0f), std::round(itemBoxSize / 4.0f))) * intScale,
            {255, 255, 255},
            24 * static_cast<unsigned int>(intScale),
            {0, 0, 0},
            0,
            true,
            true
        });
    }
    else
    {
        // Draw item info box if no item held in cursor and hovering over item
        int selectedItemIndex = getInventorySelectedIndex(mouseScreenPos);

        // If an item is hovered over, draw item info box
        if (selectedItemIndex >= 0)
        {
            drawItemInfoBox(window, selectedItemIndex, mouseScreenPos);
        }
    }
}

void InventoryGUI::drawItemInfoBox(sf::RenderWindow& window, int itemIndex, sf::Vector2f mouseScreenPos)
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
        true
    });
}