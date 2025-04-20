#include "Player/NetworkPlayer.hpp"
#include "Game.hpp"

NetworkPlayer::NetworkPlayer(pl::Vector2f position, int maxHealth)
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

    toolRotation += toolRotationVelocity * dt;
}

void NetworkPlayer::updateInRoom(float dt, const Room& room)
{
    updateAnimation(dt);
    updateMovementInRoom(dt, room, false);
    
    playerData.position.x = collisionRect.x + collisionRect.width / 2.0f;
    playerData.position.y = collisionRect.y + collisionRect.height / 2.0f;

    toolRotation += toolRotationVelocity * dt;
}

void NetworkPlayer::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const pl::Color& color) const
{
    Player::draw(window, spriteBatch, game, camera, dt, gameTime, worldSize, color);

    // Draw name
    pl::TextDrawData nameDrawData;
    nameDrawData.text = playerData.name;
    nameDrawData.position = camera.worldToScreenTransform(position - pl::Vector2f(0, 24));
    nameDrawData.centeredX = true;
    nameDrawData.centeredY = true;
    nameDrawData.color = pl::Color(255, 255, 255);
    nameDrawData.size = 9 * ResolutionHandler::getScale();
    nameDrawData.outlineColor = pl::Color(46, 34, 47);
    nameDrawData.outlineThickness = 0.6f * ResolutionHandler::getScale();

    TextDraw::drawText(window, nameDrawData);
}

PacketDataPlayerCharacterInfo NetworkPlayer::getNetworkPlayerInfo(const Camera* camera, uint64_t steamID, float dt)
{
    PacketDataPlayerCharacterInfo playerInfo = Player::getNetworkPlayerInfo(camera, steamID, dt);
    playerInfo.position = playerData.position;
    return playerInfo;
}

void NetworkPlayer::setNetworkPlayerCharacterInfo(const PacketDataPlayerCharacterInfo& info)
{
    playerData.position = info.position;
    direction = info.direction;
    speed = info.speed;

    collisionRect.x = playerData.position.x - collisionRect.width / 2.0f;
    collisionRect.y = playerData.position.y - collisionRect.height / 2.0f;

    if (direction == pl::Vector2f(0, 0))
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
    toolRotation = info.toolRotation;
    usingTool = info.usingTool;
    toolRotationVelocity = info.toolRotationVelocity;
    fishingRodCasted = info.fishingRodCasted;
    fishBitingLine = info.fishBitingLine;
    
    if (fishingRodCasted)
    {
        fishingRodBobWorldPosUnwrapped = (static_cast<pl::Vector2f>(info.fishingRodBobWorldTile) + pl::Vector2f(0.5f, 0.5f)) * TILE_SIZE_PIXELS_UNSCALED;
    }

    for (int i = 0; i < info.armour.size(); i++)
    {
        armour[i] = info.armour[i];
    }

    chunkViewRange = info.chunkViewRange;
}

void NetworkPlayer::applyWorldWrapTranslation(pl::Vector2f playerPosition, const ChunkManager& chunkManager)
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