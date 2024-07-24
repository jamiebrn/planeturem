#include "Object/WoodWall.hpp"

void WoodWall::update(float dt)
{

}

void WoodWall::draw(sf::RenderWindow& window, float dt, float alpha)
{
    TextureManager::drawSubTexture(window, {TextureType::BuildItems, position, 0, 3, {0.5, 0.5}, {255, 255, 255, alpha}}, sf::IntRect(0, 0, 16, 16));
}

void WoodWall::interact()
{

}