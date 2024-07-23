#include "Object/Bush.hpp"

void Bush::update(float dt)
{
    flash_amount = std::max(flash_amount - dt * 3, 0.0f);
}

void Bush::draw(sf::RenderWindow& window)
{
    sf::Shader* shader = Shaders::getShader(ShaderType::Flash);
    shader->setUniform("flash_amount", flash_amount);
    TextureManager::drawTexture(window, {TextureType::Bush, position + Camera::getIntegerDrawOffset(), 0, 3, {0.5, 0.7}}, shader);
}

void Bush::interact()
{
    health -= 1;
    flash_amount = 1.0f;
    if (health <= 0)
    {
        Inventory::addItem(ItemType::Wood, 1);
    }
}