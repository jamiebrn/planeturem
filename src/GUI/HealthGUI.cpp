#include "GUI/HealthGUI.hpp"

void HealthGUI::drawHealth(sf::RenderTarget& window, SpriteBatch& spriteBatch, int health, int maxHealth, float gameTime)
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

        // Heart pulsing on right-most heart
        if (health == maxHealth - i + 1)
        {
            drawData.position += (sf::Vector2f(HEART_SIZE, HEART_SIZE) * 3.0f * intScale) / 2.0f;
            drawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

            float scaleMult = (std::pow(std::sin(2 * gameTime), 2) / 6) + 1;
            drawData.scale.x *= scaleMult;
            drawData.scale.y *= scaleMult;
        }

        spriteBatch.draw(window, drawData, heartEmptyRect);

        // Draw full heart if required
        if (health > maxHealth - i)
        {
            spriteBatch.draw(window, drawData, heartRect);
        }
    }
}

void HealthGUI::drawDeadPrompt(sf::RenderTarget& window)
{
    sf::Vector2u resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    TextDrawData drawData;
    drawData.text = "You have been killed";
    drawData.size = 42 * intScale;
    drawData.colour = sf::Color(163, 34, 51);
    drawData.position = static_cast<sf::Vector2f>(resolution) / 2.0f;
    drawData.centeredX = true;
    drawData.centeredY = true;

    TextDraw::drawText(window, drawData);
}