#include "Player/Cursor.hpp"

std::array<CursorCornerPosition, 4> Cursor::tileCursorPositions;
sf::Vector2f Cursor::selectPos = {0, 0};
sf::Vector2i Cursor::selectPosTile = {0, 0};
sf::Vector2i Cursor::selectSize = {1, 1};

void Cursor::updateTileCursor(sf::RenderWindow& window, float dt, bool buildMenuOpen, ChunkManager& chunkManager)
{
    // Get mouse position in screen space and world space
    sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
    sf::Vector2f mouseWorldPos = mousePos - Camera::getDrawOffset();

    // Get selected tile position from mouse position
    selectPosTile.x = std::floor(mouseWorldPos.x / 48.0f);
    selectPosTile.y = std::floor(mouseWorldPos.y / 48.0f);

    selectPos = static_cast<sf::Vector2f>(selectPosTile) * 48.0f;

    // Default tile cursor size is 1, 1
    selectSize = sf::Vector2i(1, 1);

    // Get selected object (if any)
    BuildableObject* selectedObject = chunkManager.getChunkObject(Cursor::getSelectedChunk(), Cursor::getSelectedChunkTile());

    // If an object in world is selected, override tile cursor size and position
    if (selectedObject != nullptr && !buildMenuOpen)
    {
        // Get size of selected object and set size of tile cursor
        selectSize = ObjectDataLoader::getObjectData(selectedObject->getObjectType()).size;

        // Set position of tile cursor to object's position
        selectPos = selectedObject->getPosition() - sf::Vector2f(24.0f, 24.0f);

        // Set selected tile to new overriden tile cursor position
        selectPosTile.x = std::floor(selectPos.x / 48.0f);
        selectPosTile.y = std::floor(selectPos.y / 48.0f);
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
        cursorCorner.worldPosition.x = Helper::lerp(cursorCorner.worldPosition.x, cursorCorner.tileDestination.x * 48.0f, 25 * dt);
        cursorCorner.worldPosition.y = Helper::lerp(cursorCorner.worldPosition.y, cursorCorner.tileDestination.y * 48.0f, 25 * dt);
    }
}

void Cursor::drawTileCursor(sf::RenderWindow& window)
{
    TextureManager::drawSubTexture(window, {
        TextureType::SelectTile, tileCursorPositions[0].worldPosition + Camera::getIntegerDrawOffset(), 0, 3}, sf::IntRect(0, 0, 16, 16)); // top left

    TextureManager::drawSubTexture(window, {
        TextureType::SelectTile, tileCursorPositions[1].worldPosition + Camera::getIntegerDrawOffset(), 0, 3},
        sf::IntRect(16, 0, 16, 16)); // top right

    TextureManager::drawSubTexture(window, {
        TextureType::SelectTile, tileCursorPositions[2].worldPosition + Camera::getIntegerDrawOffset(), 0, 3},
        sf::IntRect(32, 0, 16, 16)); // bottom left

    TextureManager::drawSubTexture(window, {
        TextureType::SelectTile, tileCursorPositions[3].worldPosition + Camera::getIntegerDrawOffset(), 0, 3},
        sf::IntRect(48, 0, 16, 16)); // bottom right

}

ChunkPosition Cursor::getSelectedChunk()
{
    ChunkPosition selectedChunk;
    selectedChunk.x = std::floor(selectPosTile.x / 8.0f);
    selectedChunk.y = std::floor(selectPosTile.y / 8.0f);
    return selectedChunk;
}

sf::Vector2i Cursor::getSelectedChunkTile()
{
    sf::Vector2i selectedTile;
    selectedTile.x = ((selectPosTile.x % 8) + 8) % 8;
    selectedTile.y = ((selectPosTile.y % 8) + 8) % 8;
    return selectedTile;
}