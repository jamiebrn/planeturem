#include "Object/Tree.hpp"

void Tree::update(float dt)
{
    flash_amount = std::max(flash_amount - dt * 3, 0.0f);
}

void Tree::draw(sf::RenderWindow& window)
{
    sf::Shader* shader = Shaders::getShader(ShaderType::Flash);
    shader->setUniform("flash_amount", flash_amount);
    TextureManager::drawTexture(window, {TextureType::Tree, position + Camera::getIntegerDrawOffset(), 0, 3, {0.5, 0.85}}, shader);
}

void Tree::interact()
{
    std::cout << "interact with tree" << std::endl;
    flash_amount = 1.0f;
    health -= 1;

    if (!isAlive())
    {
        Inventory::addItem(ItemType::Wood, (rand() % 3) + 1);
    }
}