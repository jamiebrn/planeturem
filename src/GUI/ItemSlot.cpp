#include "GUI/ItemSlot.hpp"

ItemSlot::ItemSlot(pl::Vector2f position, int itemBoxSize, bool affectedByIntScale)
{
    this->position = position;

    boxSize = itemBoxSize;

    this->affectedByIntScale = affectedByIntScale;
}

void ItemSlot::update(pl::Vector2f mouseScreenPos, float dt, bool forceItemScaleUp)
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

bool ItemSlot::isHovered() const
{
    return hovered;
}

pl::Vector2f ItemSlot::getPosition()
{
    return position;
}

void ItemSlot::overrideItemScaleMult(float scale)
{
    itemScaleMult = scale;
}

void ItemSlot::draw(pl::RenderTarget& window,
                    std::optional<ItemType> itemType,
                    std::optional<int> itemAmount,
                    bool hiddenBackground,
                    bool selectHighlight,
                    std::optional<pl::Rect<int>> emptyIconTexture,
                    InventoryData* inventory)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    float positionIntScale = intScale;
    if (!affectedByIntScale)
        positionIntScale = 1.0f;

    // Draw background
    if (!hiddenBackground)
    {
        pl::Rect<int> boxRect(16, 16, 25, 25);
        pl::Vector2f origin(0, 0);
        if (selectHighlight)
        {
            boxRect = pl::Rect<int>(16, 48, 27, 27);
            origin = pl::Vector2f(1 / 27.0f, 1 / 27.0f);
        }

        pl::DrawData drawData;
        drawData.texture = TextureManager::getTexture(TextureType::UI);
        drawData.shader = Shaders::getShader(ShaderType::Default);
        drawData.position = position * positionIntScale;
        drawData.scale = {3 * intScale, 3 * intScale};
        drawData.textureRect = boxRect;
        drawData.centerRatio = origin;

        TextureManager::drawSubTexture(window, drawData);
    }

    // Draw item
    if (itemType.has_value())
    {
        pl::Vector2f itemPos = position * positionIntScale + (pl::Vector2f(std::round(boxSize / 2.0f), std::round(boxSize / 2.0f))) * intScale;
        drawItem(window, itemType.value(), itemPos, itemScaleMult);

        // If inventory data given, search for projectile counts if required
        if (inventory)
        {
            // If tool is a weapon, draw projectile count instead of amount in stack
            const ItemData& itemData = ItemDataLoader::getItemData(itemType.value());
            if (itemData.toolType >= 0)
            {
                const ToolData& toolData = ToolDataLoader::getToolData(itemData.toolType);
                if (toolData.toolBehaviourType == ToolBehaviourType::BowWeapon)
                {
                    // Get projectile count
                    int projectileCount = inventory->getProjectileCountForWeapon(itemData.toolType);

                    // Draw projectile count
                    TextDraw::drawText(window, {
                    std::to_string(projectileCount),
                    position * positionIntScale + (pl::Vector2f(std::round(boxSize / 4.0f) * 3.0f, std::round(boxSize / 4.0f) * 3.0f)) * intScale,
                    {255, 255, 255},
                    24 * static_cast<unsigned int>(intScale),
                    {0, 0, 0},
                    0,
                    true,
                    true});
                }
            }
        }
    }
    else if (emptyIconTexture.has_value())
    {
        // If no item and empty icon texture is supplied, draw empty icon in item place
        drawEmptyIconTexture(window, emptyIconTexture.value());
    }

    // Draw item count (if > 1)
    if (itemAmount.has_value())
    {
        if (itemAmount.value() > 1)
        {
            TextDraw::drawText(window, {
                std::to_string(itemAmount.value()),
                position * positionIntScale + (pl::Vector2f(std::round(boxSize / 4.0f) * 3.0f, std::round(boxSize / 4.0f) * 3.0f)) * intScale,
                {255, 255, 255},
                24 * static_cast<unsigned int>(intScale),
                {0, 0, 0},
                0,
                true,
                true});
        }
    }
}

void ItemSlot::drawEmptyIconTexture(pl::RenderTarget& window, pl::Rect<int> emptyIconTexture)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    float positionIntScale = intScale;
    if (!affectedByIntScale)
        positionIntScale = 1.0f;

    pl::Vector2f emptyIconPos = position * positionIntScale + (pl::Vector2f(std::round(boxSize / 2.0f), std::round(boxSize / 2.0f))) * intScale;

    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(TextureType::UI);
    drawData.shader = Shaders::getShader(ShaderType::Default);
    drawData.position = emptyIconPos;
    drawData.scale = pl::Vector2f(3, 3) * intScale;
    drawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    drawData.textureRect = emptyIconTexture;

    TextureManager::drawSubTexture(window, drawData);
}

void ItemSlot::drawItem(pl::RenderTarget& window, ItemType itemType, pl::Vector2f position, float scaleMult, bool centred, int alpha, float flashAmount)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    const ItemData& itemData = ItemDataLoader::getItemData(itemType);

    pl::Vector2f origin(0, 0);
    if (centred)
    {
        origin = pl::Vector2f(0.5, 0.5);
    }

    pl::Color colour(255, 255, 255, alpha);

    TextureType textureType = TextureType::Items;
    pl::Rect<int> textureRect = itemData.textureRect;
    pl::Vector2f scale(3 * scaleMult * intScale, 3 * scaleMult * intScale);

    // Draw object if item places object
    if (itemData.placesObjectType >= 0)
    {
        textureType = TextureType::Objects;

        const ObjectData& objectData = ObjectDataLoader::getObjectData(itemData.placesObjectType);

        if (objectData.rocketObjectData.has_value())
        {
            textureRect = objectData.rocketObjectData->textureRect;
        }
        else
        {
            textureRect = objectData.textureRects[0];
        }

        float objectScale = (16.0f * 3) / std::max(textureRect.width, textureRect.height) * scaleMult;
        scale = pl::Vector2f(objectScale * intScale, objectScale * intScale);
    }
    else if (itemData.toolType >= 0)
    {
        textureType = TextureType::Tools;

        const ToolData& toolData = ToolDataLoader::getToolData(itemData.toolType);

        float toolScale = std::max(4 - std::max(toolData.textureRects[0].width / 16.0f, toolData.textureRects[0].height / 16.0f), 1.0f) * scaleMult;
        scale = pl::Vector2f(toolScale * intScale, toolScale * intScale);

        textureRect = toolData.textureRects[0];
    }
    else if (itemData.armourType >= 0)
    {
        textureType = TextureType::Tools;
        const ArmourData& armourData = ArmourDataLoader::getArmourData(itemData.armourType);

        float armourScale = std::max(4 - std::max(armourData.itemTexture.width / 16.0f, armourData.itemTexture.height / 16.0f), 1.0f) * scaleMult;
        scale = pl::Vector2f(armourScale * intScale, armourScale * intScale);

        textureRect = armourData.itemTexture;
    }
    else if (itemData.projectileType >= 0)
    {
        textureType = TextureType::Tools;
        const ProjectileData& projectileData = ToolDataLoader::getProjectileData(itemData.projectileType);

        float projectileScale = std::max(4 - std::max(projectileData.textureRect.width / 16.0f, projectileData.textureRect.height / 16.0f), 1.0f) * scaleMult;
        scale = pl::Vector2f(projectileScale * intScale, projectileScale * intScale);

        textureRect = projectileData.textureRect;
    }

    pl::DrawData drawData;
    drawData.texture = TextureManager::getTexture(textureType);
    drawData.shader = Shaders::getShader(ShaderType::Default);
    drawData.position = position;
    drawData.scale = scale;
    drawData.centerRatio = origin;
    drawData.colour = colour;
    drawData.textureRect = textureRect;

    if (flashAmount > 0.0f)
    {
        drawData.shader = Shaders::getShader(ShaderType::Flash);
        drawData.shader->setUniform1f("flash_amount", flashAmount);
    }
    
    // Draw item / tool / object
    TextureManager::drawSubTexture(window, drawData);
}