#include "GUI/Base/ColorWheel.hpp"

ColorWheel::ColorWheel(const GUIInputState& inputState, ElementID id, int x, int y, int size, float value, pl::Color& currentColor)
    : GUIElement(id, textSize)
{
    this->x = x;
    this->y = y;
    this->size = size;
    
    active = false;
    clickedAway = false;

    if (inputState.activeElement == id)
    {
        active = true;

        if (InputManager::isControllerActive())
        {
            static constexpr float CONTROLLER_STEP = 0.02f;

            pl::Color hsvColor = Helper::convertRGBtoHSV(currentColor);

            pl::Vector2f currentSelectedPosition;
            currentSelectedPosition.x = std::cos(hsvColor.r / 180.0f * M_PI) * hsvColor.g;
            currentSelectedPosition.y = std::sin(hsvColor.r / 180.0f * M_PI) * hsvColor.g;

            currentSelectedPosition.x += InputManager::getActionAxisActivation(InputAction::WALK_LEFT, InputAction::WALK_RIGHT) * CONTROLLER_STEP;
            currentSelectedPosition.y += InputManager::getActionAxisActivation(InputAction::WALK_UP, InputAction::WALK_DOWN) * CONTROLLER_STEP;
            
            float angle = std::atan2(currentSelectedPosition.y, -currentSelectedPosition.x) + M_PI;
            angle = fmod(360.0f - angle / M_PI * 180.0f, 360.0f);
            printf("%f\n", angle);
            currentColor = Helper::convertHSVtoRGB(angle, std::min(currentSelectedPosition.getLength(), 1.0f), value);
        }
    }

    CollisionCircle circle(x, y, size);
    if (circle.isPointColliding(inputState.mouseX, inputState.mouseY) && inputState.leftMousePressed)
    {
        float angle = std::atan2(inputState.mouseY - y, x - inputState.mouseX) + M_PI;
        angle = fmod(360.0f - angle / M_PI * 180.0f, 360.0f);
        float distance = pl::Vector2f(inputState.mouseX - x, inputState.mouseY - y).getLength();

        currentColor = Helper::convertHSVtoRGB(angle, distance / size, value);

        active = true;
    }

    this->currentColor = currentColor;
    this->value = value;
}

bool ColorWheel::isActive() const
{
    return active;
}

bool ColorWheel::hasClickedAway() const
{
    return clickedAway;
}

void ColorWheel::draw(pl::RenderTarget& window)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();

    pl::VertexArray wheel;

    int divisions = DebugOptions::colorWheelDivisions;
    for (int i = 0; i < divisions; i++)
    {
        wheel.addVertex(pl::Vertex(pl::Vector2f(std::cos(i / static_cast<float>(divisions) * 2 * M_PI) * (size * intScale),
            std::sin(i / static_cast<float>(divisions) * 2 * M_PI) * (size * intScale)) + pl::Vector2f(x, y),
            Helper::convertHSVtoRGB(i / static_cast<float>(divisions) * 360, 1, value)), false);
        wheel.addVertex(pl::Vertex(pl::Vector2f(std::cos((i + 1) / static_cast<float>(divisions) * 2 * M_PI) * (size * intScale),
            std::sin((i + 1) / static_cast<float>(divisions) * 2 * M_PI) * (size * intScale)) + pl::Vector2f(x, y),
            Helper::convertHSVtoRGB(fmod((i + 1) / static_cast<float>(divisions) * 360, 360), 1, value)), false);
        wheel.addVertex(pl::Vertex(pl::Vector2f(x, y), pl::Color(255 * value, 255 * value, 255 * value)), false);
    }

    window.draw(wheel, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    wheel.clear();

    // Find position of current selector from RGB color
    pl::Color hsvColor = Helper::convertRGBtoHSV(currentColor);
    float angle = hsvColor.r / 360.0f * 2 * M_PI;
    
    pl::Vector2f hsvPosition;
    hsvPosition.x = x + std::cos(angle) * size * hsvColor.g;
    hsvPosition.y = y + std::sin(angle) * size * hsvColor.g;

    wheel.addQuad(pl::Rect<float>(hsvPosition - pl::Vector2f(3, 3) * intScale, pl::Vector2f(6, 6) * intScale), pl::Color(0, 0, 0), pl::Rect<float>());

    wheel.addQuad(pl::Rect<float>(pl::Vector2f(x - size - 20 * intScale, y - 5 * intScale), pl::Vector2f(10, 10) * intScale), currentColor, pl::Rect<float>());

    window.draw(wheel, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);
}

pl::Rect<int> ColorWheel::getBoundingBox() const
{
    return pl::Rect<int>(x - size, y - size, size * 2, size * 2);
}