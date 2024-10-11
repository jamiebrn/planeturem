#include "Entity/Boss/BossBenjaminCrow.hpp"
#include "Game.hpp"

BossBenjaminCrow::BossBenjaminCrow(sf::Vector2f position)
{
    this->position = position;

    idleAnim.create(6, 48, 64, 64, 144, 0.1);

    flyingHeight = 20.0f;
    behaviourState = BossBenjaminState::Idle;
}

void BossBenjaminCrow::update(Game& game, sf::Vector2f playerPos, float dt)
{
    // Update animation
    idleAnim.update(dt);

    // Update movement
    direction.x = playerPos.x - position.x;
    direction.y = playerPos.y - position.y;
    
    direction = Helper::normaliseVector(direction);

    position += direction * MOVE_SPEED * dt;
}

void BossBenjaminCrow::draw(sf::RenderTarget& window, SpriteBatch& spriteBatch)
{
    // Draw shadow
    TextureDrawData drawData;
    drawData.type = TextureType::Entities;

    drawData.position = Camera::worldToScreenTransform(position);

    float scale = ResolutionHandler::getScale();
    drawData.scale = sf::Vector2f(scale, scale);

    drawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

    spriteBatch.draw(window, drawData, sf::IntRect(75, 208, 26, 16));

    // Draw bird
    sf::Vector2f worldPos(position.x, position.y - flyingHeight);
    drawData.position = Camera::worldToScreenTransform(worldPos);

    // Flip if required
    if (direction.x < 0)
    {
        drawData.scale.x *= -1;
    }

    drawData.centerRatio = sf::Vector2f(0.5f, 1.0f);

    switch (behaviourState)
    {
        case BossBenjaminState::Idle:
        {
            spriteBatch.draw(window, drawData, idleAnim.getTextureRect());
            break;
        }
    }
}

void BossBenjaminCrow::handleWorldWrap(sf::Vector2f positionDelta)
{
    position += positionDelta;
}