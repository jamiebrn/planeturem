#include "Player/Cursor.hpp"

std::array<CursorCornerPosition, 4> Cursor::cursorCornerPositions;
std::array<AnimatedTexture, 4> Cursor::cursorAnimatedTextures = {
    AnimatedTexture(6, 22, 22, 0, 0, 0.08),      // top left
    AnimatedTexture(6, 22, 22, 0, 22, 0.08),     // top right
    AnimatedTexture(6, 22, 22, 0, 44, 0.08),     // bottom left
    AnimatedTexture(6, 22, 22, 0, 66, 0.08)      // bottom right
};
sf::Vector2f Cursor::selectPos = {0, 0};
sf::Vector2i Cursor::selectPosTile = {0, 0};
sf::Vector2i Cursor::selectSize = {1, 1};

void Cursor::updateTileCursor(sf::RenderWindow& window, float dt, bool buildMenuOpen, int worldSize, ChunkManager& chunkManager)
{
    // Get mouse position in screen space and world space
    sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
    sf::Vector2f mouseWorldPos = mousePos - Camera::getDrawOffset();

    // Get tile size
    float tileSize = ResolutionHandler::getTileSize();

    // Get selected tile position from mouse position
    selectPosTile.x = std::floor(mouseWorldPos.x / tileSize);
    selectPosTile.y = std::floor(mouseWorldPos.y / tileSize);

    selectPos = static_cast<sf::Vector2f>(selectPosTile) * tileSize;

    // Default tile cursor size is 1, 1
    selectSize = sf::Vector2i(1, 1);

    // Get whether an object is selected at cursor position
    std::optional<BuildableObject>& selectedObjectOptional = chunkManager.getChunkObject(Cursor::getSelectedChunk(worldSize), Cursor::getSelectedChunkTile());

    // If an object in world is selected, override tile cursor size and position
    if (selectedObjectOptional.has_value() && !buildMenuOpen)
    {
        // Get object selected
        BuildableObject& selectedObject = selectedObjectOptional.value();

        // Get size of selected object and set size of tile cursor
        selectSize = ObjectDataLoader::getObjectData(selectedObject.getObjectType()).size;

        // Set position of tile cursor to object's position
        selectPos = selectedObject.getPosition() - sf::Vector2f(tileSize / 2.0f, tileSize / 2.0f);

        // Set selected tile to new overriden tile cursor position
        selectPosTile.x = std::floor(selectPos.x / tileSize);
        selectPosTile.y = std::floor(selectPos.y / tileSize);
    }
    else if (buildMenuOpen)
    {
        // Override cursor size to size of currently selected object, if in build menu
        selectSize = (ObjectDataLoader::getObjectData(BuildGUI::getSelectedObject()).size);
    }

    // Set tile cursor corner tile positions
    cursorCornerPositions[0].tileDestination = selectPosTile;
    cursorCornerPositions[1].tileDestination = selectPosTile + sf::Vector2i(selectSize.x - 1, 0);
    cursorCornerPositions[2].tileDestination = selectPosTile + sf::Vector2i(0, selectSize.y - 1);
    cursorCornerPositions[3].tileDestination = selectPosTile + sf::Vector2i(selectSize.x - 1, selectSize.y - 1);

    // Lerp tile cursor corners to desination positions if build menu open
    if (buildMenuOpen)
    {
        for (CursorCornerPosition& cursorCorner : cursorCornerPositions)
        {
            cursorCorner.worldPosition.x = Helper::lerp(cursorCorner.worldPosition.x, cursorCorner.tileDestination.x * tileSize, 25 * dt);
            cursorCorner.worldPosition.y = Helper::lerp(cursorCorner.worldPosition.y, cursorCorner.tileDestination.y * tileSize, 25 * dt);
        }

        // Set cursor animation to freeze at index 0
        for (int cursorCornerIdx = 0; cursorCornerIdx < cursorAnimatedTextures.size(); cursorCornerIdx++)
        {
            cursorAnimatedTextures[cursorCornerIdx].setFrame(0);
        }
    }
    else
    {
        // Immediately set cursor position to destination position (no lerp)
        setCursorCornersToDestination();

        // Update animations
        // Update top left then set all others to same frame
        cursorAnimatedTextures[0].update(dt);
        int cursorTopLeftAnimationFrame = cursorAnimatedTextures[0].getFrame();

        for (int cursorCornerIdx = 1; cursorCornerIdx < cursorAnimatedTextures.size(); cursorCornerIdx++)
        {
            cursorAnimatedTextures[cursorCornerIdx].setFrame(cursorTopLeftAnimationFrame);
        }
    }

}

void Cursor::setCursorCornersToDestination()
{
    // Get tile size
    float tileSize = ResolutionHandler::getTileSize();

    for (CursorCornerPosition& cursorCorner : cursorCornerPositions)
    {
        cursorCorner.worldPosition.x = cursorCorner.tileDestination.x * tileSize;
        cursorCorner.worldPosition.y = cursorCorner.tileDestination.y * tileSize;
    }
}

void Cursor::drawTileCursor(sf::RenderWindow& window)
{
    float scale = ResolutionHandler::getScale();

    static constexpr float cursorTextureOrigin = 3.0f / 22.0f;

    for (int cursorCornerIdx = 0; cursorCornerIdx < cursorCornerPositions.size(); cursorCornerIdx++)
    {
        TextureManager::drawSubTexture(window, {
            TextureType::SelectTile, cursorCornerPositions[cursorCornerIdx].worldPosition + Camera::getIntegerDrawOffset(), 0, {scale, scale},
            {cursorTextureOrigin, cursorTextureOrigin}}, cursorAnimatedTextures[cursorCornerIdx].getTextureRect());
    }
}

ChunkPosition Cursor::getSelectedChunk(int worldSize)
{
    ChunkPosition selectedChunk;
    selectedChunk.x = ((static_cast<int>(std::floor(selectPosTile.x / 8.0f)) % worldSize) + worldSize) % worldSize;
    selectedChunk.y = ((static_cast<int>(std::floor(selectPosTile.y / 8.0f)) % worldSize) + worldSize) % worldSize;
    return selectedChunk;
}

sf::Vector2i Cursor::getSelectedChunkTile()
{
    sf::Vector2i selectedTile;
    selectedTile.x = ((selectPosTile.x % 8) + 8) % 8;
    selectedTile.y = ((selectPosTile.y % 8) + 8) % 8;
    return selectedTile;
}