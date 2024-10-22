#include "GUI/HealthGUI.hpp"

void HealthGUI::drawHealth(sf::RenderTarget& window, SpriteBatch& spriteBatch, int health, int maxHealth, float gameTime)
{
    sf::Vector2u resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    const sf::IntRect heartEmptyRect = sf::IntRect(192, 16, 16, 16);
    const sf::IntRect heartRect = sf::IntRect(176, 16, 16, 16);

    int maxHearts = std::ceil(static_cast<float>(maxHealth) / HEALTH_PER_HEART);
    int fullHearts = std::ceil(static_cast<float>(health) / HEALTH_PER_HEART);

    // Draw hearts
    for (int i = 1; i <= maxHearts; i++)
    {
        int xPos = HEART_X_PADDING + (HEART_SIZE * 3) * i + HEART_SPACING * (i - 1);

        TextureDrawData drawData;
        drawData.type = TextureType::UI;
        drawData.position = sf::Vector2f(resolution.x - xPos  * intScale, HEART_Y_PADDING  * intScale);
        drawData.scale = sf::Vector2f(3, 3) * intScale;

        bool useProgressShader = false;

        // Heart pulsing on right-most heart and use progress shader
        if (fullHearts == maxHearts - i + 1)
        {
            drawData.position += (sf::Vector2f(HEART_SIZE, HEART_SIZE) * 3.0f * intScale) / 2.0f;
            drawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

            float scaleMult = (std::pow(std::sin(2 * gameTime), 2) / 6) + 1;
            drawData.scale.x *= scaleMult;
            drawData.scale.y *= scaleMult;
            
            useProgressShader = true;
        }

        spriteBatch.draw(window, drawData, heartEmptyRect);

        // Draw full heart if required
        if (fullHearts > maxHearts - i)
        {
            if (useProgressShader)
            {
                float heartProgress = static_cast<float>(health % HEALTH_PER_HEART) / HEALTH_PER_HEART;
                if (heartProgress == 0.0f)
                {
                    heartProgress = 1.0f;
                }

                sf::Shader* progressShader = Shaders::getShader(ShaderType::Progress);
                progressShader->setUniform("progress", heartProgress);
                progressShader->setUniform("spriteSheetSize", static_cast<sf::Glsl::Vec2>(TextureManager::getTextureSize(TextureType::UI)));
                progressShader->setUniform("textureRect", sf::Glsl::Vec4(heartRect.left, heartRect.top, heartRect.width, heartRect.height));
                spriteBatch.draw(window, drawData, heartRect, ShaderType::Progress);
            }
            else
            {
                spriteBatch.draw(window, drawData, heartRect);
            }
        }
    }

    // Draw text showing health
    TextDrawData textDrawData;
    textDrawData.text = std::to_string(health) + " / " + std::to_string(maxHealth);
    textDrawData.position = sf::Vector2f(resolution.x, (HEART_Y_PADDING * 1.5f + HEART_SIZE * 3) * intScale);
    textDrawData.size = 24 * intScale;
    textDrawData.colour = sf::Color(255, 255, 255);
    textDrawData.containOnScreenX = true;
    textDrawData.containPaddingRight = HEART_X_PADDING * intScale;

    TextDraw::drawText(window, textDrawData);
}

void HealthGUI::drawDeadPrompt(sf::RenderTarget& window)
{
    sf::Vector2u resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    TextDrawData drawData;
    drawData.text = "You have been killed";
    drawData.size = 42 * intScale;
    drawData.colour = sf::Color(220, 30, 55);
    drawData.position = static_cast<sf::Vector2f>(resolution) / 2.0f;
    drawData.centeredX = true;
    drawData.centeredY = true;

    TextDraw::drawText(window, drawData);
}