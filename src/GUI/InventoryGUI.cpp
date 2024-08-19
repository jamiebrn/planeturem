#include "GUI/InventoryGUI.hpp"

sf::Vector2f InventoryGUI::screenPos;

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
            pickedUpItemCount += std::min(itemCount.second, amount);

            // Take from inventory
            Inventory::takeItemAtIndex(itemIndex, amount);
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
    sf::Vector2f itemBoxPosition = sf::Vector2f(10, 10) * static_cast<float>(intScale) + sf::Vector2f(10, 10) * static_cast<float>(intScale);

    for (int itemIndex = 0; itemIndex < Inventory::getData().size(); itemIndex++)
    {
        CollisionRect itemPickUpRect;
        
        itemPickUpRect.width = 80 * intScale;
        itemPickUpRect.height = 80 * intScale;
        itemPickUpRect.x = itemBoxPosition.x;
        itemPickUpRect.y = itemBoxPosition.y;

        // Test if in item box area
        if (itemPickUpRect.isPointInRect(mouseScreenPos.x, mouseScreenPos.y))
        {
            return itemIndex;
        }

        // Increment box position
        itemBoxPosition.x += (80 + 20) * intScale;
        if (itemBoxPosition.x > 10 * intScale + 10 * intScale + 7 * 100 * intScale)
        {
            itemBoxPosition.x = 10 * intScale + 10 * intScale;
            itemBoxPosition.y += 100 * intScale;
        }
    }

    // Default case
    return -1;
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

void InventoryGUI::draw(sf::RenderWindow& window)
{
    // Get resolution
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();
    int intScale = ResolutionHandler::getResolutionIntegerScale();

    // Draw background
    sf::RectangleShape background({800 * static_cast<float>(intScale), 400 * static_cast<float>(intScale)});

    // background.setOrigin({400, 200});
    background.setPosition({10 * static_cast<float>(intScale), 10 * static_cast<float>(intScale)});
    background.setFillColor({40, 40, 40, 130});

    // window.draw(background);

    // Draw items

    sf::Vector2f itemBoxPosition = background.getPosition() + sf::Vector2f(10, 10) * static_cast<float>(intScale);

    for (const std::optional<ItemCount>& itemSlot : Inventory::getData())
    {
        sf::RectangleShape itemBackground({80 * static_cast<float>(intScale), 80 * static_cast<float>(intScale)});

        // itemBackground.setOrigin({40, 40});
        // itemBackground.setPosition(itemBoxPosition);
        // itemBackground.setFillColor({40, 40, 40, 140});

        // window.draw(itemBackground);

        TextureManager::drawSubTexture(window, {
                TextureType::UI,
                itemBoxPosition + sf::Vector2f(40, 40),
                0,
                {5, 5},
                {0.5, 0.5}
            }, sf::IntRect(0, 16, 16, 16));

        if (itemSlot.has_value())
        {
            const ItemCount& itemCount = itemSlot.value();

            TextureManager::drawSubTexture(window, {
                TextureType::Items,
                itemBoxPosition + sf::Vector2f(40, 40),
                0,
                {3, 3},
                {0.5, 0.5}
            }, ItemDataLoader::getItemData(itemCount.first).textureRect);

            TextDraw::drawText(window, {std::to_string(itemCount.second), itemBoxPosition + sf::Vector2f(70, 70), {255, 255, 255}, 24, {0, 0, 0}, 0, true, true});
        }
        
        itemBoxPosition.x += (itemBackground.getSize().x + 20) * intScale;
        if (itemBoxPosition.x > background.getPosition().x + 10 * intScale + 7 * 100 * intScale)
        {
            itemBoxPosition.x = background.getPosition().x + 10 * intScale;
            itemBoxPosition.y += 100 * intScale;
        }
    }

    // Draw picked up item at cursor
    if (isItemPickedUp)
    {
        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));

        TextureManager::drawSubTexture(window, {
            TextureType::Items,
            mousePos,
            0,
            {3, 3},
            {0.5, 0.5}
        }, ItemDataLoader::getItemData(pickedUpItem).textureRect);

        TextDraw::drawText(window, {
            std::to_string(pickedUpItemCount),
            mousePos + sf::Vector2f(8, 8) * 3.0f,
            {255, 255, 255},
            24,
            {0, 0, 0},
            0,
            true,
            true
        });
    }
}