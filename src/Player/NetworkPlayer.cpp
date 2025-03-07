#include "Player/NetworkPlayer.hpp"

NetworkPlayer::NetworkPlayer(sf::Vector2f position, int maxHealth)
    : Player(position, maxHealth)
{

}

void NetworkPlayer::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize, const sf::Color& color) const
{
    Player::draw(window, spriteBatch, game, camera, dt, gameTime, worldSize, color);

    // Draw name
    TextDrawData nameDrawData;
    nameDrawData.text = playerData.name;
    nameDrawData.position = camera.worldToScreenTransform(position - sf::Vector2f(0, 24));
    nameDrawData.centeredX = true;
    nameDrawData.centeredY = true;
    nameDrawData.colour = sf::Color(255, 255, 255);
    nameDrawData.size = 9 * ResolutionHandler::getScale();
    nameDrawData.outlineColour = sf::Color(46, 34, 47);
    nameDrawData.outlineThickness = 0.6f * ResolutionHandler::getScale();
    TextDraw::drawText(window, nameDrawData);
}

void NetworkPlayer::setNetworkPlayerInfo(const PacketDataPlayerCharacterInfo& info, std::string steamName, sf::Vector2f playerPosition, const ChunkManager& chunkManager)
{
    position = chunkManager.translatePositionAroundWorld(sf::Vector2f(info.positionX, info.positionY), playerPosition);
    
    collisionRect.x = position.x - collisionRect.width / 2.0f;
    collisionRect.y = position.y - collisionRect.height / 2.0f;

    if (info.animationFrame < idleAnimation.getFrameCount())
    {
        // Idle
        direction = sf::Vector2f(0, 0);
        idleAnimation.setFrame(info.animationFrame);
    }
    else
    {
        // Running
        direction = sf::Vector2f(1, 0); // non-zero
        runAnimation.setFrame(info.animationFrame - idleAnimation.getFrameCount());
    }

    flippedTexture = info.flipped;
    playerYScaleMult = info.yScaleMult;

    onWater = info.onWater;

    equippedTool = info.toolType;
    toolRotation = info.toolRotation;
    fishingRodCasted = info.fishingRodCasted;
    fishBitingLine = info.fishBitingLine;
    
    if (fishingRodCasted)
    {
        fishingRodBobWorldPos = (static_cast<sf::Vector2f>(info.fishingRodBobWorldTile) + sf::Vector2f(0.5f, 0.5f)) * TILE_SIZE_PIXELS_UNSCALED;
        fishingRodBobWorldPos = chunkManager.translatePositionAroundWorld(fishingRodBobWorldPos, playerPosition);
    }

    armour = info.armour;

    playerData.name = steamName;
}

PlayerData NetworkPlayer::getPlayerDataUpdated()
{
    PlayerData newPlayerData = playerData;
    newPlayerData.position = position;
    return newPlayerData;
}

PlayerData& NetworkPlayer::getPlayerData()
{
    return playerData;
}

void NetworkPlayer::setPlayerData(const PlayerData& playerData)
{
    this->playerData = playerData;
}