#include "Object/Bush.hpp"

void Bush::draw(sf::RenderWindow& window)
{
    TextureManager::drawTexture(window, {TextureType::Bush, position + Camera::getIntegerDrawOffset(), 0, 3, {0.5, 0.7}});
}