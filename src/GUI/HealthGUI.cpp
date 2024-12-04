#include "GUI/HealthGUI.hpp"
#include "Player/Player.hpp"

void HealthGUI::drawHealth(sf::RenderTarget& window, SpriteBatch& spriteBatch, const Player& player, float gameTime, const std::vector<std::string>& extraInfo)
{
    sf::Vector2u resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    static const sf::IntRect heartEmptyRect = sf::IntRect(192, 16, 16, 16);
    static const sf::IntRect heartRect = sf::IntRect(176, 16, 16, 16);

    int maxHearts = std::ceil(static_cast<float>(player.getMaxHealth()) / HEALTH_PER_HEART);
    int fullHearts = std::ceil(player.getHealth() / HEALTH_PER_HEART);

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
                float heartProgress = static_cast<float>(static_cast<int>(player.getHealth()) % HEALTH_PER_HEART) / HEALTH_PER_HEART;
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
    textDrawData.text = std::to_string(static_cast<int>(player.getHealth())) + " / " + std::to_string(player.getMaxHealth());
    textDrawData.position = sf::Vector2f(resolution.x, (HEART_Y_PADDING * 1.5f + HEART_SIZE * 3) * intScale);
    textDrawData.size = 24 * intScale;
    textDrawData.colour = sf::Color(255, 255, 255);
    textDrawData.containOnScreenX = true;
    textDrawData.containPaddingRight = HEART_X_PADDING * intScale;

    TextDraw::drawText(window, textDrawData);

    for (const std::string& string : extraInfo)
    {
        textDrawData.position.y += 24 * intScale;
        textDrawData.text = string;

        TextDraw::drawText(window, textDrawData);
    }

    static constexpr int statPadding = 6;
    static constexpr int statInfoSpacing = 16 * 3 + 10;

    sf::Vector2f statPosition(resolution.x - (statPadding + 16 * 3) * intScale, textDrawData.position.y + 40 * intScale);

    TextureDrawData statDrawData;
    statDrawData.position = statPosition;
    statDrawData.type = TextureType::UI;
    statDrawData.scale = sf::Vector2f(3, 3) * intScale;

    textDrawData.position.y = statPosition.y + 16 / 2 * 3 * intScale;
    textDrawData.centeredY = true;
    textDrawData.containPaddingRight = (statPadding + 16 * 3 + 10) * intScale;

    sf::Shader* progressCircleShader = Shaders::getShader(ShaderType::ProgressCircle);
    progressCircleShader->setUniform("cropOption", 1);
    progressCircleShader->setUniform("spriteSheetSize", static_cast<sf::Glsl::Vec2>(TextureManager::getTextureSize(TextureType::UI)));

    if (player.getHealthConsumableTimer() > 0)
    {
        static const sf::IntRect healthRegenTextureRect(192, 48, 16, 16);

        float progress = 1.0f - (player.getHealthConsumableTimer() / player.getHealthConsumableTimerMax());
        progressCircleShader->setUniform("progress", progress);
        progressCircleShader->setUniform("textureRect", sf::Glsl::Vec4(healthRegenTextureRect.left, healthRegenTextureRect.top,
            healthRegenTextureRect.width, healthRegenTextureRect.height));
        
        spriteBatch.draw(window, statDrawData, healthRegenTextureRect, ShaderType::ProgressCircle);

        textDrawData.text = std::to_string(static_cast<int>(player.getHealthConsumableTimer()));
        TextDraw::drawText(window, textDrawData);

        statDrawData.position.y += statInfoSpacing * intScale;
        textDrawData.position.y += statInfoSpacing * intScale;
    }
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