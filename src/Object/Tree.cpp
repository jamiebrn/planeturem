#include "Object/Tree.hpp"

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
}