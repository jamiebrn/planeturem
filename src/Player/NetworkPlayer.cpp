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
    
    updateAnimation(dt);
    
    damageCooldownTimer = std::max(damageCooldownTimer - dt, 0.0f);

    switch (playerData.locationState.getGameState())
    {
        case GameState::OnPlanet:
            updateMovement(dt, game.getChunkManager(playerData.locationState.getPlanetType()), false);
            break;
        case GameState::InStructure:
            updateMovementInRoom(dt, game.getStructureRoomPool(playerData.locationState.getPlanetType())
                .getRoom(playerData.locationState.getInStructureID()), false);
            break;
        case GameState::InRoomDestination:
            updateMovementInRoom(dt, game.getRoomDestination(playerData.locationState.getRoomDestType()), false);
            break;
    }
    
    playerData.position.x = collisionRect.x + collisionRect.width / 2.0f;
    playerData.position.y = collisionRect.y + collisionRect.height / 2.0f;
    position = playerData.position;

    toolRotation += toolRotationVelocity * dt;
}

void NetworkPlayer::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera& camera, float dt, float gameTime, int worldSize,
    const pl::Color& color) const
{
    draw(window, spriteBatch, game, &camera, dt, gameTime, worldSize, color, true);
}

void NetworkPlayer::draw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, Game& game, const Camera* camera, float dt, float gameTime, int worldSize,
    const pl::Color& color, bool drawName) const
{
    Player::draw(window, spriteBatch, game, camera, dt, gameTime, worldSize, color);

    // Draw name
    if (drawName)
    {
        pl::TextDrawData nameDrawData;
        nameDrawData.text = playerData.name;
        nameDrawData.position = position - pl::Vector2f(0, 24);
        if (camera)
        {
            nameDrawData.position = camera->worldToScreenTransform(nameDrawData.position, worldSize);
        }
        nameDrawData.centeredX = true;
        nameDrawData.centeredY = true;
        nameDrawData.color = pl::Color(255, 255, 255);
        nameDrawData.size = 9 * ResolutionHandler::getScale();
        nameDrawData.outlineColor = pl::Color(46, 34, 47);
        nameDrawData.outlineThickness = 0.6f * ResolutionHandler::getScale();
    
        TextDraw::drawText(window, nameDrawData);
    }
}

void NetworkPlayer::setNetworkPlayerCharacterInfo(const PacketDataPlayerCharacterInfo& info)
{
    playerData.position = info.position;
    position = info.position;
    direction = info.direction;
    speed = info.speed;

    health = info.health;

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
        fishingRodBobWorldPos = (static_cast<pl::Vector2f>(info.fishingRodBobWorldTile) + pl::Vector2f(0.5f, 0.5f)) * TILE_SIZE_PIXELS_UNSCALED;
    }

    for (int i = 0; i < info.armour.size(); i++)
    {
        armour[i] = info.armour[i];
    }

    chunkViewRange = info.chunkViewRange;
}

// void NetworkPlayer::applyWorldWrapTranslation(pl::Vector2f playerPosition, const ChunkManager& chunkManager)
// {
//     position = chunkManager.translatePositionAroundWorld(playerData.position, playerPosition);
    
//     if (fishingRodCasted)
//     {
//         fishingRodBobWorldPos = chunkManager.translatePositionAroundWorld(fishingRodBobWorldPosUnwrapped, playerPosition);
//     }
// }

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

    bodyColor = playerData.bodyColor;
    skinColor = playerData.skinColor;
}

const ChunkViewRange& NetworkPlayer::getChunkViewRange()
{
    return chunkViewRange;
}