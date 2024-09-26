#include "Player/Cursor.hpp"

std::array<CursorCornerPosition, 4> Cursor::cursorCornerPositions;
std::array<AnimatedTexture, 4> Cursor::cursorAnimatedTextures = {
    AnimatedTexture(6, 22, 22, 0, 0, 0.08),      // top left
    AnimatedTexture(6, 22, 22, 0, 22, 0.08),     // top right
    AnimatedTexture(6, 22, 22, 0, 44, 0.08),     // bottom left
    AnimatedTexture(6, 22, 22, 0, 66, 0.08)      // bottom right
};
CursorDrawState Cursor::drawState = CursorDrawState::Hidden;
sf::Vector2f Cursor::selectPos = {0, 0};
sf::Vector2i Cursor::selectPosTile = {0, 0};
sf::Vector2i Cursor::selectSize = {1, 1};

void Cursor::updateTileCursor(sf::RenderWindow& window,
                              float dt,
                              ChunkManager& chunkManager,
                              const CollisionRect& playerCollisionRect,
                              ItemType heldItemType,
                              ToolType toolType)
{
    // Get mouse position in screen space and world space
    sf::Vector2f mouseWorldPos = getMouseWorldPos(window);

    // Get selected tile position from mouse position
    selectPosTile.x = std::floor(mouseWorldPos.x / TILE_SIZE_PIXELS_UNSCALED);
    selectPosTile.y = std::floor(mouseWorldPos.y / TILE_SIZE_PIXELS_UNSCALED);

    selectPos = static_cast<sf::Vector2f>(selectPosTile) * TILE_SIZE_PIXELS_UNSCALED;

    // Default tile cursor size is 1, 1
    selectSize = sf::Vector2i(1, 1);

    // Set drawing to hidden by default
    drawState = CursorDrawState::Hidden;

    if (heldItemType >= 0)
    {
        // Get held item data
        const ItemData& itemData = ItemDataLoader::getItemData(heldItemType);

        // Override cursor size if object is being placed
        if (itemData.placesObjectType >= 0)
        {
            updateTileCursorPlaceObject(itemData.placesObjectType);
        }
        else if (itemData.placesLand)
        {
            updateTileCursorPlaceLand(window);
        }
        else if (toolType >= 0)
        {
            // Get current tool data
            const ToolData& toolData = ToolDataLoader::getToolData(toolType);

            switch (toolData.toolBehaviourType)
            {
                case ToolBehaviourType::Pickaxe:
                    updateTileCursorToolPickaxe(window, dt, chunkManager, playerCollisionRect);
                    break;
                case ToolBehaviourType::FishingRod:
                    updateTileCursorToolFishingRod(window, dt, chunkManager);
                    break;
            }
        }
        else
        {
            updateTileCursorNoItem(dt, chunkManager);
        }
    }
    else
    {
        updateTileCursorNoItem(dt, chunkManager);
    }

    // Set tile cursor corner tile positions
    if (drawState == CursorDrawState::Tile)
    {
        cursorCornerPositions[0].worldPositionDestination = static_cast<sf::Vector2f>(selectPosTile) * TILE_SIZE_PIXELS_UNSCALED;
        cursorCornerPositions[1].worldPositionDestination = static_cast<sf::Vector2f>(selectPosTile + sf::Vector2i(selectSize.x - 1, 0)) * TILE_SIZE_PIXELS_UNSCALED;
        cursorCornerPositions[2].worldPositionDestination = static_cast<sf::Vector2f>(selectPosTile + sf::Vector2i(0, selectSize.y - 1)) * TILE_SIZE_PIXELS_UNSCALED;
        cursorCornerPositions[3].worldPositionDestination = static_cast<sf::Vector2f>(selectPosTile + sf::Vector2i(selectSize.x - 1, selectSize.y - 1)) * TILE_SIZE_PIXELS_UNSCALED;
    }

    setCursorCornersToDestination();
}

void Cursor::updateTileCursorPlaceObject(ObjectType objectType)
{
    selectSize = ObjectDataLoader::getObjectData(objectType).size;

    // Set cursor animation to freeze at index 0
    for (int cursorCornerIdx = 0; cursorCornerIdx < cursorAnimatedTextures.size(); cursorCornerIdx++)
    {
        cursorAnimatedTextures[cursorCornerIdx].setFrame(0);
    }

    drawState = CursorDrawState::Tile;
}

void Cursor::updateTileCursorPlaceLand(sf::RenderWindow& window)
{
    // Set cursor animation to freeze at index 0
    for (int cursorCornerIdx = 0; cursorCornerIdx < cursorAnimatedTextures.size(); cursorCornerIdx++)
    {
        cursorAnimatedTextures[cursorCornerIdx].setFrame(0);
    }

    drawState = CursorDrawState::Tile;
}

void Cursor::updateTileCursorToolPickaxe(sf::RenderWindow& window, float dt, ChunkManager& chunkManager, const CollisionRect& playerCollisionRect)
{
    int worldSize = chunkManager.getWorldSize();
    sf::Vector2f mouseWorldPos = getMouseWorldPos(window);

    // Get entity selected at cursor position (if any)
    Entity* selectedEntity = chunkManager.getSelectedEntity(Cursor::getSelectedChunk(worldSize), mouseWorldPos);
    
    // If entity is selected and in main world state, set size of cursor to entity size
    if (selectedEntity != nullptr)
    {
        selectPos = selectedEntity->getPosition();

        sf::Vector2f selectSizeFloat = selectedEntity->getSize();

        // Set tile cursor corner tile positions
        cursorCornerPositions[0].worldPositionDestination = selectPos - selectSizeFloat / 2.0f;
        cursorCornerPositions[1].worldPositionDestination = selectPos + sf::Vector2f(selectSizeFloat.x, -selectSizeFloat.y) / 2.0f;
        cursorCornerPositions[2].worldPositionDestination = selectPos + sf::Vector2f(-selectSizeFloat.x, selectSizeFloat.y) / 2.0f;
        cursorCornerPositions[3].worldPositionDestination = selectPos + selectSizeFloat / 2.0f;

        // Immediately set cursor position to destination position (no lerp)
        setCursorCornersToDestination();

        // Set cursor animation to freeze at index 0
        for (int cursorCornerIdx = 0; cursorCornerIdx < cursorAnimatedTextures.size(); cursorCornerIdx++)
        {
            cursorAnimatedTextures[cursorCornerIdx].setFrame(0);
        }

        // Set dynamic draw to true as is not a tile-based selection
        drawState = CursorDrawState::Dynamic;

        // Entity is selected, so should not attempt to find object
        return;
    }

    // Get object selected at cursor position (if any)
    std::optional<BuildableObject>& selectedObjectOptional = chunkManager.getChunkObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile());

    // If an object in world is selected, override tile cursor size and position
    if (selectedObjectOptional.has_value())
    {
        // Test if can destroy selected object - don't draw cursor if cannot destroy
        if (!chunkManager.canDestroyObject(getSelectedChunk(worldSize), getSelectedChunkTile(), playerCollisionRect))
        {
            drawState = CursorDrawState::Hidden;
            return;
        }

        // Get object selected
        BuildableObject& selectedObject = selectedObjectOptional.value();

        // Get size of selected object and set size of tile cursor
        selectSize = ObjectDataLoader::getObjectData(selectedObject.getObjectType()).size;

        // Set position of tile cursor to object's position
        selectPos = selectedObject.getPosition() - sf::Vector2f(TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f);

        // Set selected tile to new overriden tile cursor position
        selectPosTile.x = std::floor(selectPos.x / TILE_SIZE_PIXELS_UNSCALED);
        selectPosTile.y = std::floor(selectPos.y / TILE_SIZE_PIXELS_UNSCALED);

        // Set draw state to tile
        drawState = CursorDrawState::Tile;

        // Update cursor animation
        updateTileCursorAnimation(dt);

        return;
    }

    // Neither enemy nor object is selected, so hide cursor
    drawState = CursorDrawState::Hidden;
}

void Cursor::updateTileCursorToolFishingRod(sf::RenderWindow& window, float dt, ChunkManager& chunkManager)
{
    ChunkPosition selectedChunk = getSelectedChunk(chunkManager.getWorldSize());
    sf::Vector2i selectedTile = getSelectedChunkTile();

    // Test whether can fish on selected tile
    // Must have no object + be water
    const std::optional<BuildableObject>& selectedObject = chunkManager.getChunkObject(selectedChunk, selectedTile);
    int tileType = chunkManager.getChunkTileType(selectedChunk, selectedTile);

    if (!selectedObject.has_value() && tileType == 0)
    {
        drawState = CursorDrawState::Tile;

        updateTileCursorAnimation(dt);

        return;
    }

    // Default case - tile cannot be fished on
    drawState = CursorDrawState::Hidden;
}

void Cursor::updateTileCursorNoItem(float dt, ChunkManager& chunkManager)
{
    int worldSize = chunkManager.getWorldSize();

    std::optional<BuildableObject>& selectedObjectOptional = chunkManager.getChunkObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile());

    // If an object in world is selected, override tile cursor size and position
    if (selectedObjectOptional.has_value())
    {
        // Get object selected
        BuildableObject& selectedObject = selectedObjectOptional.value();

        if (selectedObject.isInteractable())
        {
            // Get size of selected object and set size of tile cursor
            selectSize = ObjectDataLoader::getObjectData(selectedObject.getObjectType()).size;

            // Set position of tile cursor to object's position
            selectPos = selectedObject.getPosition() - sf::Vector2f(TILE_SIZE_PIXELS_UNSCALED / 2.0f, TILE_SIZE_PIXELS_UNSCALED / 2.0f);

            // Set selected tile to new overriden tile cursor position
            selectPosTile.x = std::floor(selectPos.x / TILE_SIZE_PIXELS_UNSCALED);
            selectPosTile.y = std::floor(selectPos.y / TILE_SIZE_PIXELS_UNSCALED);

            // Set draw state to tile
            drawState = CursorDrawState::Tile;

            // Update cursor animation
            updateTileCursorAnimation(dt);

            return;
        }
    }

    // No object selected
    drawState = CursorDrawState::Hidden;
}

void Cursor::updateTileCursorAnimation(float dt)
{
    cursorAnimatedTextures[0].update(dt);
    int cursorTopLeftAnimationFrame = cursorAnimatedTextures[0].getFrame();

    for (int cursorCornerIdx = 1; cursorCornerIdx < cursorAnimatedTextures.size(); cursorCornerIdx++)
    {
        cursorAnimatedTextures[cursorCornerIdx].setFrame(cursorTopLeftAnimationFrame);
    }
}

void Cursor::setCursorCornersToDestination()
{
    for (CursorCornerPosition& cursorCorner : cursorCornerPositions)
    {
        cursorCorner.worldPosition.x = cursorCorner.worldPositionDestination.x;
        cursorCorner.worldPosition.y = cursorCorner.worldPositionDestination.y;
    }
}

void Cursor::drawCursor(sf::RenderWindow& window)
{
    switch(drawState)
    {
        case CursorDrawState::Hidden:
            break;
        case CursorDrawState::Tile:
            drawTileCursor(window);
            break;
        case CursorDrawState::Dynamic:
            drawDynamicCursor(window);
            break;
    }
}

void Cursor::drawTileCursor(sf::RenderWindow& window)
{
    float scale = ResolutionHandler::getScale();

    static constexpr float cursorTextureOrigin = 3.0f / 22.0f;

    for (int cursorCornerIdx = 0; cursorCornerIdx < cursorCornerPositions.size(); cursorCornerIdx++)
    {
        TextureManager::drawSubTexture(window, {
            TextureType::SelectTile, Camera::worldToScreenTransform(cursorCornerPositions[cursorCornerIdx].worldPosition), 0, {scale, scale},
            {cursorTextureOrigin, cursorTextureOrigin}}, cursorAnimatedTextures[cursorCornerIdx].getTextureRect());
    }
}

void Cursor::drawDynamicCursor(sf::RenderWindow& window)
{
    float scale = ResolutionHandler::getScale();

    static const std::array<sf::Vector2f, 4> cursorTextureOrigin = {
        sf::Vector2f(3.0f / 22.0f, 3.0f / 22.0f),
        sf::Vector2f(18.0f / 22.0f, 3.0f / 22.0f),
        sf::Vector2f(3.0f / 22.0f, 18.0f / 22.0f),
        sf::Vector2f(18.0f / 22.0f, 18.0f / 22.0f)
    };

    for (int cursorCornerIdx = 0; cursorCornerIdx < cursorCornerPositions.size(); cursorCornerIdx++)
    {
        TextureManager::drawSubTexture(window, {
            TextureType::SelectTile, Camera::worldToScreenTransform(cursorCornerPositions[cursorCornerIdx].worldPosition), 0, {scale, scale},
            cursorTextureOrigin[cursorCornerIdx]}, cursorAnimatedTextures[cursorCornerIdx].getTextureRect());
    }
}

ChunkPosition Cursor::getSelectedChunk(int worldSize)
{
    ChunkPosition selectedChunk;
    selectedChunk.x = ((static_cast<int>(std::floor(selectPosTile.x / CHUNK_TILE_SIZE)) % worldSize) + worldSize) % worldSize;
    selectedChunk.y = ((static_cast<int>(std::floor(selectPosTile.y / CHUNK_TILE_SIZE)) % worldSize) + worldSize) % worldSize;
    return selectedChunk;
}

sf::Vector2i Cursor::getSelectedChunkTile()
{
    sf::Vector2i selectedTile;
    selectedTile.x = static_cast<int>((selectPosTile.x % static_cast<int>(CHUNK_TILE_SIZE)) + CHUNK_TILE_SIZE) % static_cast<int>(CHUNK_TILE_SIZE);
    selectedTile.y = static_cast<int>((selectPosTile.y % static_cast<int>(CHUNK_TILE_SIZE)) + CHUNK_TILE_SIZE) % static_cast<int>(CHUNK_TILE_SIZE);
    return selectedTile;
}

sf::Vector2i Cursor::getSelectedWorldTile(int worldSize)
{
    ChunkPosition selectedChunk = getSelectedChunk(worldSize);
    sf::Vector2i selectedWorldTile = getSelectedChunkTile();
    selectedWorldTile.x += selectedChunk.x * CHUNK_TILE_SIZE;
    selectedWorldTile.y += selectedChunk.y * CHUNK_TILE_SIZE;
    return selectedWorldTile;
}

sf::Vector2f Cursor::getMouseWorldPos(sf::RenderWindow& window)
{
    sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
    return Camera::screenToWorldTransform(mousePos);
}

void Cursor::setCursorHidden(bool hidden)
{
    if (hidden)
        drawState = CursorDrawState::Hidden;
}

void Cursor::handleWorldWrap(sf::Vector2f positionDelta)
{    
    // Move all cursor corners to wrap around world
    for (CursorCornerPosition& cursorCorner : cursorCornerPositions)
    {
        cursorCorner.worldPositionDestination += positionDelta;
        cursorCorner.worldPosition += positionDelta;
    }
}