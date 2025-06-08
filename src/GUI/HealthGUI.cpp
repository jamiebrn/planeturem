#include "GUI/HealthGUI.hpp"
#include "Player/Player.hpp"

void HealthGUI::drawHealth(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, const Player& player, float gameTime, const std::vector<std::string>& extraInfo)
{
    pl::Vector2<uint32_t> resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    static const pl::Rect<int> heartEmptyRect = pl::Rect<int>(192, 16, 16, 16);
    static const pl::Rect<int> heartRect = pl::Rect<int>(176, 16, 16, 16);

    int maxHearts = std::ceil(static_cast<float>(player.getMaxHealth()) / HEALTH_PER_HEART);
    int fullHearts = std::ceil(player.getHealth() / HEALTH_PER_HEART);

    int heartXPos = 0;

    // Draw hearts
    for (int i = 1; i <= maxHearts; i++)
    {
        if ((maxHearts - i + 1) % HEARTS_PER_ROW == 0)
        {
            heartXPos = 0;
        }

        heartXPos++;

        int xPos = HEART_X_PADDING + (HEART_SIZE * 3) * heartXPos + HEART_X_SPACING * (heartXPos - 1);
        int yPos = HEART_Y_PADDING + std::floor((maxHearts - i) / HEARTS_PER_ROW) * HEART_Y_SPACING;

        pl::DrawData drawData;
        drawData.texture = TextureManager::getTexture(TextureType::UI);
        drawData.shader = Shaders::getShader(ShaderType::Default);
        drawData.position = pl::Vector2f(resolution.x - xPos  * intScale, yPos * intScale);
        drawData.scale = pl::Vector2f(3, 3) * intScale;
        drawData.textureRect = heartEmptyRect;
        drawData.vertexPixelClamp = false;

        bool useProgressShader = false;

        // Heart pulsing on right-most heart and use progress shader
        if (fullHearts == maxHearts - i + 1)
        {
            drawData.position += (pl::Vector2f(HEART_SIZE, HEART_SIZE) * 3.0f * intScale) / 2.0f;
            drawData.centerRatio = pl::Vector2f(0.5f, 0.5f);

            float scaleMult = (std::pow(std::sin(2 * gameTime), 2) / 6) + 1;
            drawData.scale.x *= scaleMult;
            drawData.scale.y *= scaleMult;
            
            if ((maxHearts - i + 1) * HEALTH_PER_HEART > player.getHealth())
            {
                useProgressShader = true;
            }
        }

        spriteBatch.draw(window, drawData);

        // Draw full heart if required
        if (fullHearts > maxHearts - i)
        {
            drawData.textureRect = heartRect;

            if (useProgressShader)
            {
                float heartProgress = static_cast<float>(static_cast<int>(player.getHealth()) % HEALTH_PER_HEART) / HEALTH_PER_HEART;    

                drawData.shader = Shaders::getShader(ShaderType::Progress);
                drawData.shader->setUniform1f("progress", heartProgress);
                drawData.shader->setUniform2f("spriteSheetSize", drawData.texture->getWidth(), drawData.texture->getHeight());
                drawData.shader->setUniform4f("textureRect", heartRect.x, heartRect.y, heartRect.width, heartRect.height);
            }

            spriteBatch.draw(window, drawData);
        }
    }

    // Draw text showing health
    pl::TextDrawData textDrawData;
    textDrawData.text = std::to_string(static_cast<int>(player.getHealth())) + " / " + std::to_string(player.getMaxHealth());
    textDrawData.position = pl::Vector2f(resolution.x, (HEART_Y_PADDING + (std::floor(maxHearts / HEARTS_PER_ROW) + 1) * HEART_Y_SPACING) * intScale);
    textDrawData.size = 24 * intScale;
    textDrawData.outlineColor = pl::Color(46, 34, 47);
    textDrawData.outlineThickness = 2 * intScale;
    textDrawData.color = pl::Color(255, 255, 255);
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

    pl::Vector2f statPosition(resolution.x - (statPadding + 16 * 3) * intScale, textDrawData.position.y + 40 * intScale);

    pl::DrawData statDrawData;
    statDrawData.position = statPosition;
    statDrawData.texture = TextureManager::getTexture(TextureType::UI);
    statDrawData.shader = Shaders::getShader(ShaderType::ProgressCircle);
    statDrawData.scale = pl::Vector2f(3, 3) * intScale;

    textDrawData.position.y = statPosition.y + 16 / 2 * 3 * intScale;
    textDrawData.centeredY = true;
    textDrawData.containPaddingRight = (statPadding + 16 * 3 + 10) * intScale;

    statDrawData.shader->setUniform1i("cropOption", 1);
    statDrawData.shader->setUniform2f("spriteSheetSize", statDrawData.texture->getWidth(), statDrawData.texture->getHeight());

    if (player.getHealthConsumableTimer() > 0)
    {
        static const pl::Rect<int> healthRegenTextureRect(192, 48, 16, 16);
            
        statDrawData.textureRect = healthRegenTextureRect;

        float progress = 1.0f - (player.getHealthConsumableTimer() / player.getHealthConsumableTimerMax());
        statDrawData.shader->setUniform1f("progress", progress);
        statDrawData.shader->setUniform4f("textureRect", healthRegenTextureRect.x, healthRegenTextureRect.y,
            healthRegenTextureRect.width, healthRegenTextureRect.height);
        
        spriteBatch.draw(window, statDrawData);

        textDrawData.text = std::to_string(static_cast<int>(player.getHealthConsumableTimer()));
        TextDraw::drawText(window, textDrawData);

        statDrawData.position.y += statInfoSpacing * intScale;
        textDrawData.position.y += statInfoSpacing * intScale;
    }
}

void HealthGUI::drawDeadPrompt(pl::RenderTarget& window)
{
    pl::Vector2<uint32_t> resolution = ResolutionHandler::getResolution();
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    pl::TextDrawData drawData;
    drawData.text = "You have been killed";
    drawData.size = 42 * intScale;
    drawData.outlineColor = pl::Color(46, 34, 47);
    drawData.outlineThickness = 3 * intScale;
    drawData.color = pl::Color(220, 30, 55);
    drawData.position = static_cast<pl::Vector2f>(resolution) / 2.0f;
    drawData.centeredX = true;
    drawData.centeredY = true;

    TextDraw::drawText(window, drawData);
}