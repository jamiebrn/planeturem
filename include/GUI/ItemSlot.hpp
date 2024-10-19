#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

#include "Core/ResolutionHandler.hpp"
#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"

#include "Player/InventoryData.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"
#include "Data/ArmourData.hpp"
#include "Data/ArmourDataLoader.hpp"

// Does not store / control what is in item slot
class ItemSlot
{
public:
    ItemSlot() = default;
    ItemSlot(sf::Vector2f position, int itemBoxSize, bool affectedByIntScale = true);

    void update(sf::Vector2f mouseScreenPos, float dt, bool forceItemScaleUp = false);

    bool isHovered() const;

    sf::Vector2f getPosition();

    void overrideItemScaleMult(float scale);

    void draw(sf::RenderTarget& window,
              std::optional<ItemType> itemType = std::nullopt,
              std::optional<int> itemAmount = std::nullopt,
              bool hiddenBackground = false,
              bool selectHighlight = false,
              std::optional<sf::IntRect> emptyIconTexture = std::nullopt,
              InventoryData* inventory = nullptr // give pointer to inventory if amount of projectiles etc required to be drawn
              );
    
    static void drawItem(sf::RenderTarget& window, ItemType itemType, sf::Vector2f position, float scaleMult = 1.0f, bool centred = true, int alpha = 255);

private:
    void drawEmptyIconTexture(sf::RenderTarget& window, sf::IntRect emptyIconTexture);

private:
    sf::Vector2f position;
    
    int boxSize = 0;

    static constexpr float ITEM_HOVERED_SCALE = 1.3f;
    static constexpr float ITEM_HOVERED_SCALE_LERP_WEIGHT = 15.0f;
    float itemScaleMult = 1.0f;

    bool hovered = false;

    // Graphically selected (highlight around)
    // bool selected = false;

    bool affectedByIntScale = true;

};