#include "GUI/HealthGUI.hpp"

void HealthGUI::drawHealth(sf::RenderTarget& window, SpriteBatch& spriteBatch, int health, int maxHealth)
{
    sf::Vector2u resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    const sf::IntRect heartEmptyRect = sf::IntRect(192, 16, 16, 16);
    const sf::IntRect heartRect = sf::IntRect(176, 16, 16, 16);

    // Draw hearts
    for (int i = 1; i <= maxHealth; i++)
    {
        int xPos = HEART_X_PADDING + (HEART_SIZE * 3 * intScale) * i + HEART_SPACING * (i - 1);

        TextureDrawData drawData;
        drawData.type = TextureType::UI;
        drawData.position = sf::Vector2f(resolution.x - xPos  * intScale, HEART_Y_PADDING  * intScale);
        drawData.scale = sf::Vector2f(3, 3) * intScale;

        spriteBatch.draw(window, drawData, heartEmptyRect);

        // Draw full heart if required
        if (health > maxHealth - i)
        {
            spriteBatch.draw(window, drawData, heartRect);
        }
    }
}