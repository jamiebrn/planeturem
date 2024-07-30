#include "Player/Cursor.hpp"

std::array<CursorCornerPosition, 4> Cursor::tileCursorPositions;
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
    tileCursorPositions[0].tileDestination = selectPosTile;
    tileCursorPositions[1].tileDestination = selectPosTile + sf::Vector2i(selectSize.x - 1, 0);
    tileCursorPositions[2].tileDestination = selectPosTile + sf::Vector2i(0, selectSize.y - 1);
    tileCursorPositions[3].tileDestination = selectPosTile + sf::Vector2i(selectSize.x - 1, selectSize.y - 1);

    // Lerp tile cursor corners to desination positions
    for (CursorCornerPosition& cursorCorner : tileCursorPositions)
    {
        cursorCorner.worldPosition.x = Helper::lerp(cursorCorner.worldPosition.x, cursorCorner.tileDestination.x * tileSize, 25 * dt);
        cursorCorner.worldPosition.y = Helper::lerp(cursorCorner.worldPosition.y, cursorCorner.tileDestination.y * tileSize, 25 * dt);
    }
}

void Cursor::drawTileCursor(sf::RenderWindow& window)
{
    float scale = ResolutionHandler::getScale();

    TextureManager::drawSubTexture(window, {
        TextureType::SelectTile, tileCursorPositions[0].worldPosition + Camera::getIntegerDrawOffset(), 0, {scale, scale}}, sf::IntRect(0, 0, 16, 16)); // top left

    TextureManager::drawSubTexture(window, {
        TextureType::SelectTile, tileCursorPositions[1].worldPosition + Camera::getIntegerDrawOffset(), 0, {scale, scale}},
        sf::IntRect(16, 0, 16, 16)); // top right

    TextureManager::drawSubTexture(window, {
        TextureType::SelectTile, tileCursorPositions[2].worldPosition + Camera::getIntegerDrawOffset(), 0, {scale, scale}},
        sf::IntRect(32, 0, 16, 16)); // bottom left

    TextureManager::drawSubTexture(window, {
        TextureType::SelectTile, tileCursorPositions[3].worldPosition + Camera::getIntegerDrawOffset(), 0, {scale, scale}},
        sf::IntRect(48, 0, 16, 16)); // bottom right

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