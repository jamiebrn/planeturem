#include "GUI/ItemSlot.hpp"

ItemSlot::ItemSlot(sf::Vector2f position, int itemBoxSize, bool affectedByIntScale)
{
    this->position = position;

    boxSize = itemBoxSize;

    this->affectedByIntScale = affectedByIntScale;
}

void ItemSlot::update(sf::Vector2f mouseScreenPos, float dt, bool forceItemScaleUp)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    CollisionRect itemSlotRect;
        
    itemSlotRect.width = boxSize * intScale;
    itemSlotRect.height = boxSize * intScale;
    itemSlotRect.x = position.x * intScale;
    itemSlotRect.y = position.y * intScale;

    // Test if in item box area
    hovered = itemSlotRect.isPointInRect(mouseScreenPos.x, mouseScreenPos.y);

    // Update scale if hovered
    if (hovered || forceItemScaleUp)
    {
        // Scale up
        itemScaleMult = Helper::lerp(itemScaleMult, ITEM_HOVERED_SCALE, ITEM_HOVERED_SCALE_LERP_WEIGHT * dt);
    }
    else
    {
        // Scale down to 1.0f
        itemScaleMult = Helper::lerp(itemScaleMult, 1.0f, ITEM_HOVERED_SCALE_LERP_WEIGHT * dt);
    }
}

bool ItemSlot::isHovered()
{
    return hovered;
}

sf::Vector2f ItemSlot::getPosition()
{
    return position;
}

void ItemSlot::overrideItemScaleMult(float scale)
{
    itemScaleMult = scale;
}

void ItemSlot::draw(sf::RenderTarget& window,
                    std::optional<ItemType> itemType,
                    std::optional<int> itemAmount,
                    bool hiddenBackground,
                    bool selectHighlight)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    float positionIntScale = intScale;
    if (!affectedByIntScale)
        positionIntScale = 1.0f;

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
            position * positionIntScale,
            0,
            {3 * intScale, 3 * intScale},
            origin
        }, boxRect);
    }

    // Draw item
    if (itemType.has_value())
    {
        sf::Vector2f itemPos = position * positionIntScale + (sf::Vector2f(std::round(boxSize / 2.0f), std::round(boxSize / 2.0f))) * intScale;
        drawItem(window, itemType.value(), itemPos, itemScaleMult);
    }

    // Draw item count (if > 1)
    if (itemAmount.has_value())
    {
        if (itemAmount.value() > 1)
        {
            TextDraw::drawText(window, {
                std::to_string(itemAmount.value()),
                position * positionIntScale + (sf::Vector2f(std::round(boxSize / 4.0f) * 3.0f, std::round(boxSize / 4.0f) * 3.0f)) * intScale,
                {255, 255, 255},
                24 * static_cast<unsigned int>(intScale),
                {0, 0, 0},
                0,
                true,
                true});
        }
    }
}

void ItemSlot::drawItem(sf::RenderTarget& window, ItemType itemType, sf::Vector2f position, float scaleMult, bool centred, int alpha)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    sf::Vector2f origin(0, 0);
    if (centred)
    {
        origin = sf::Vector2f(0.5, 0.5);
    }

    sf::Color colour(255, 255, 255, alpha);

    // Draw object if item places object
    if (itemData.placesObjectType >= 0)
    {
        const ObjectData& objectData = ObjectDataLoader::getObjectData(itemData.placesObjectType);

        float objectScale = std::max(4 - std::max(objectData.textureRects[0].width / 16.0f, objectData.textureRects[0].height / 16.0f), 1.0f) * scaleMult;

        // Draw object
        TextureManager::drawSubTexture(window, {
            TextureType::Objects,
            position,
            0,
            {objectScale * intScale, objectScale * intScale},
            origin,
            colour
        }, objectData.textureRects[0]);
    }
    else if (itemData.toolType >= 0)
    {
        // Draw tool
        const ToolData& toolData = ToolDataLoader::getToolData(itemData.toolType);

        float objectScale = std::max(4 - std::max(toolData.textureRects[0].width / 16.0f, toolData.textureRects[0].height / 16.0f), 1.0f) * scaleMult;

        TextureManager::drawSubTexture(window, {
            TextureType::Tools,
            position,
            0,
            {objectScale * intScale, objectScale * intScale},
            origin,
            colour
        }, toolData.textureRects[0]);
    }
    else
    {
        // Draw as normal
        TextureManager::drawSubTexture(window, {
            TextureType::Items,
            position,
            0,
            {3 * scaleMult * intScale, 3 * scaleMult * intScale},
            origin,
            colour
        }, itemData.textureRect);
    }
}