#pragma once


#include <optional>

#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/DrawData.hpp>
#include <Graphics/TextDrawData.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/ResolutionHandler.hpp"
#include "Core/TextureManager.hpp"
#include "Core/TextDraw.hpp"
#include "Core/CollisionRect.hpp"
#include "Core/Shaders.hpp"

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
    ItemSlot(pl::Vector2f position, int itemBoxSize, bool affectedByIntScale = true);

    void update(pl::Vector2f mouseScreenPos, float dt, bool forceItemScaleUp = false);

    bool isHovered() const;

    pl::Vector2f getPosition();

    inline int getItemBoxSize() {return boxSize;}

    void overrideItemScaleMult(float scale);

    void draw(pl::RenderTarget& window,
              pl::SpriteBatch& spriteBatch,
              std::optional<ItemType> itemType = std::nullopt,
              std::optional<int> itemAmount = std::nullopt,
              bool hiddenBackground = false,
              bool selectHighlight = false,
              std::optional<pl::Rect<int>> emptyIconTexture = std::nullopt,
              InventoryData* inventory = nullptr // give pointer to inventory if amount of projectiles etc required to be drawn
              );
    
    static void drawItem(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, ItemType itemType, pl::Vector2f position, float scaleMult = 1.0f,
        bool centred = true, int alpha = 255, float flashAmount = 0.0f);

private:
    void drawEmptyIconTexture(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, pl::Rect<int> emptyIconTexture);

private:
    pl::Vector2f position;
    
    int boxSize = 0;

    static constexpr float ITEM_HOVERED_SCALE = 1.3f;
    static constexpr float ITEM_HOVERED_SCALE_LERP_WEIGHT = 15.0f;
    float itemScaleMult = 1.0f;

    bool hovered = false;

    // Graphically selected (highlight around)
    // bool selected = false;

    bool affectedByIntScale = true;

};