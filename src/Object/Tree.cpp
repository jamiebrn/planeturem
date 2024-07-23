#include "Object/Tree.hpp"

void Tree::draw(sf::RenderWindow& window)
{
    TextureManager::drawTexture(window, {TextureType::Tree, position + Camera::getIntegerDrawOffset(), 0, 3, {0.5, 0.9}});
}