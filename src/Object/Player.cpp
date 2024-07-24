#include "Object/Player.hpp"

void Player::update(float dt)
{
    sf::Vector2f direction;
    direction.x = sf::Keyboard::isKeyPressed(sf::Keyboard::D) - sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    direction.y = sf::Keyboard::isKeyPressed(sf::Keyboard::S) - sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length > 0)
        direction /= length;

    position += direction * 300.0f * dt;
}

void Player::draw(sf::RenderWindow& window, float dt, int alpha)
{
    TextureManager::drawTexture(window, {TextureType::Player, position + Camera::getIntegerDrawOffset(), 0, 3, {0.5, 0.7}});
}