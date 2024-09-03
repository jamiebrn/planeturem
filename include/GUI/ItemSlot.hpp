#pragma once

#include <SFML/Graphics.hpp>
#include <optional>

#include "Core/ResolutionHandler.hpp"
#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"

#include "Data/typedefs.hpp"
#include "Data/ItemData.hpp"
#include "Data/ItemDataLoader.hpp"
#include "Data/ObjectData.hpp"
#include "Data/ObjectDataLoader.hpp"
#include "Data/ToolData.hpp"
#include "Data/ToolDataLoader.hpp"

// Does not store / control what is in item slot
class ItemSlot
{
public:
    ItemSlot() = default;
    ItemSlot(sf::Vector2f position, int itemBoxSize, bool affectedByIntScale = true);

    void update(sf::Vector2f mouseScreenPos, float dt, bool forceItemScaleUp = false);

    bool isHovered();

    sf::Vector2f getPosition();

    void overrideItemScaleMult(float scale);

    void draw(sf::RenderWindow& window,
              std::optional<ItemType> itemType = std::nullopt,
              std::optional<int> itemAmount = std::nullopt,
              bool hiddenBackground = false,
              bool selectHighlight = false);
    
    static void drawItem(sf::RenderWindow& window, ItemType itemType, sf::Vector2f position, float scaleMult = 1.0f, bool centred = true);

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