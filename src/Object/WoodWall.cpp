#include "Object/WoodWall.hpp"

void WoodWall::update(float dt)
{

}

void WoodWall::draw(sf::RenderWindow& window, float dt, const sf::Color& color)
{
    TextureManager::drawSubTexture(window, {
        TextureType::BuildItems, position + Camera::getIntegerDrawOffset(), 0, 3, {0.5, 0.5}, color
        }, sf::IntRect(0, 0, 16, 16));
}

void WoodWall::drawGUI(sf::RenderWindow& window, float dt, const sf::Color& color)
{
    TextureManager::drawSubTexture(window, {
        TextureType::BuildItems, position, 0, 3, {0.5, 0.5}, color
        }, sf::IntRect(0, 0, 16, 16));
}

void WoodWall::interact()
{

}