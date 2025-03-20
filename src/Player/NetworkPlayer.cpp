#include "Player/NetworkPlayer.hpp"
#include "Game.hpp"

NetworkPlayer::NetworkPlayer(sf::Vector2f position, int maxHealth)
    : Player(position, maxHealth)
{

}

void NetworkPlayer::updateNetworkPlayer(float dt, Game& game)
{
    if (playerData.locationState.isNull())
    {
        return;
    }

    switch (playerData.locationState.getGameState())
    {
        case GameState::OnPlanet:
            updateOnPlanet(dt, game.getChunkManager(playerData.locationState.getPlanetType()));
            break;
        case GameState::InStructure:
            updateInRoom(dt, game.getStructureRoomPool(playerData.locationState.getPlanetType())
                .getRoom(playerData.locationState.getInStructureID()));
            break;
        case GameState::InRoomDestination:
            updateInRoom(dt, game.getRoomDestination(playerData.locationState.getRoomDestType()));
            break;
    }
}

void NetworkPlayer::updateOnPlanet(float dt, ChunkManager& chunkManager)
{
    updateAnimation(dt);
    updateMovement(dt, chunkManager, false);
    playerData.position.x = collisionRect.x + collisionRect.width / 2.0f;
    playerData.position.y = collisionRect.y + collisionRect.height / 2.0f;
}

void NetworkPlayer::updateInRoom(float dt, const Room& room)
{
    updateAnimation(dt);
    updateMovementInRoom(dt, room, false);
    playerData.position.x = collisionRect.x + collisionRect.width / 2.0f;
    playerData.position.y = collisionRect.y + collisionRect.height / 2.0f;
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

PacketDataPlayerCharacterInfo NetworkPlayer::getNetworkPlayerInfo(const Camera* camera, uint64_t steamID)
{
    PacketDataPlayerCharacterInfo playerInfo = Player::getNetworkPlayerInfo(camera, steamID);
    playerInfo.position = playerData.position;
    return playerInfo;
}

void NetworkPlayer::setNetworkPlayerCharacterInfo(const PacketDataPlayerCharacterInfo& info)
{
    // position = chunkManager.translatePositionAroundWorld(sf::Vector2f(info.positionX, info.positionY), playerPosition);
    playerData.position = info.position;
    direction = info.direction;
    speed = info.speed;

    collisionRect.x = playerData.position.x - collisionRect.width / 2.0f;
    collisionRect.y = playerData.position.y - collisionRect.height / 2.0f;

    if (direction == sf::Vector2f(0, 0))
    {
        idleAnimation.setFrame(info.animationFrame);
        idleAnimation.setFrameTick(info.animationFrameTick);
        idleAnimation.update(info.pingTime);
    }
    else
    {
        runAnimation.setFrame(info.animationFrame);
        runAnimation.setFrameTick(info.animationFrameTick);
        runAnimation.update(info.pingTime);
    }


    flippedTexture = info.flipped;
    playerYScaleMult = info.yScaleMult;

    onWater = info.onWater;

    inRocket = info.inRocket;

    equippedTool = info.toolType;
    fishingRodCasted = info.fishingRodCasted;
    fishBitingLine = info.fishBitingLine;
    
    if (fishingRodCasted)
    {
        fishingRodBobWorldPosUnwrapped = (static_cast<sf::Vector2f>(info.fishingRodBobWorldTile) + sf::Vector2f(0.5f, 0.5f)) * TILE_SIZE_PIXELS_UNSCALED;
    }

    usingTool = info.usingTool;

    if (usingTool)
    {
        rotationTweenID = info.toolRotTweenID;
        TweenData<float> tweenData = info.toolTweenData;
        tweenData.value = &toolRotation;
        toolTweener.overwriteTweenData(rotationTweenID, tweenData);
    }

    armour = info.armour;

    chunkViewRange = info.chunkViewRange;
}

void NetworkPlayer::applyWorldWrapTranslation(sf::Vector2f playerPosition, const ChunkManager& chunkManager)
{
    position = chunkManager.translatePositionAroundWorld(playerData.position, playerPosition);
    
    if (fishingRodCasted)
    {
        fishingRodBobWorldPos = chunkManager.translatePositionAroundWorld(fishingRodBobWorldPosUnwrapped, playerPosition);
    }
}

PlayerData& NetworkPlayer::getPlayerData()
{
    return playerData;
}

void NetworkPlayer::setPlayerData(const PlayerData& playerData)
{
    // Retain ping location
    std::string pingLocation = playerData.pingLocation;

    this->playerData = playerData;

    this->playerData.pingLocation = pingLocation;
}

const ChunkViewRange& NetworkPlayer::getChunkViewRange()
{
    return chunkViewRange;
}