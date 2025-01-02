#include "Core/InputManager.hpp"

float InputManager::controllerAxisDeadzone = 0.0f;
bool InputManager::controllerIsActive = false;

std::unordered_map<InputAction, sf::Keyboard::Key> InputManager::keyBindings;
std::unordered_map<InputAction, sf::Mouse::Button> InputManager::mouseBindings;
std::unordered_map<InputAction, JoystickAxisWithDirection> InputManager::controllerAxisBindings;
std::unordered_map<InputAction, unsigned int> InputManager::controllerButtonBindings;

std::unordered_map<InputAction, float> InputManager::inputActivation;
std::unordered_map<InputAction, float> InputManager::inputActivationLastFrame;

void InputManager::bindKey(InputAction action, std::optional<sf::Keyboard::Key> key)
{
    if (!key.has_value())
    {
        if (keyBindings.count(action) > 0)
        {
            keyBindings.erase(action);
        }
        return;
    }
    
    keyBindings[action] = key.value();
}

void InputManager::bindMouseButton(InputAction action, std::optional<sf::Mouse::Button> button)
{
    if (!button.has_value())
    {
        if (mouseBindings.count(action) > 0)
        {
            mouseBindings.erase(action);
        }
        return;
    }

    mouseBindings[action] = button.value();
}

void InputManager::bindControllerAxis(InputAction action, std::optional<JoystickAxisWithDirection> axisWithDirection)
{
    if (!axisWithDirection.has_value())
    {
        if (controllerAxisBindings.count(action) > 0)
        {
            controllerAxisBindings.erase(action);
        }
        return;
    }

    controllerAxisBindings[action] = axisWithDirection.value();
}

void InputManager::bindControllerButton(InputAction action, std::optional<unsigned int> button)
{
    if (!button.has_value())
    {
        if (controllerButtonBindings.count(action) > 0)
        {
            controllerButtonBindings.erase(action);
        }
        return;
    }

    controllerButtonBindings[action] = button.value();
}

void InputManager::setControllerAxisDeadzone(float deadzone)
{
    controllerAxisDeadzone = deadzone;
}

void InputManager::update()
{
    inputActivationLastFrame = inputActivation;

    inputActivation.clear();

    // Check key bindings
    for (auto iter = keyBindings.begin(); iter != keyBindings.end(); iter++)
    {
        float activation = 0.0f;
        if (sf::Keyboard::isKeyPressed(iter->second))
        {
            activation = 1.0f;
            controllerIsActive = false;
        }

        inputActivation[iter->first] = std::max(inputActivation[iter->first], activation);
    }

    // Check mouse bindings
    for (auto iter = mouseBindings.begin(); iter != mouseBindings.end(); iter++)
    {
        float activation = 0.0f;
        if (sf::Mouse::isButtonPressed(iter->second))
        {
            activation = 1.0f;
            controllerIsActive = false;
        }

        inputActivation[iter->first] = std::max(inputActivation[iter->first], activation);
    }

    // Check controller axis bindings
    for (auto iter = controllerAxisBindings.begin(); iter != controllerAxisBindings.end(); iter++)
    {
        float activation = 0.0f;
        float axisPosition = sf::Joystick::getAxisPosition(1, iter->second.axis) / 100.0f;

        if (std::abs(axisPosition) >= controllerAxisDeadzone)
        {
            switch (iter->second.direction)
            {
                case JoystickAxisDirection::POSITIVE:
                {
                    activation = std::max(axisPosition, 0.0f);
                    break;
                }
                case JoystickAxisDirection::NEGATIVE:
                {
                    activation = std::min(axisPosition, 0.0f);
                    break;
                }
            }

            controllerIsActive = true;
        }

        inputActivation[iter->first] = std::max(inputActivation[iter->first], std::abs(activation));
    }

    // Check controller button bindings
    for (auto iter = controllerButtonBindings.begin(); iter != controllerButtonBindings.end(); iter++)
    {
        float activation = 0.0f;
        if (sf::Joystick::isButtonPressed(1, iter->second))
        {
            activation = 1.0f;
            controllerIsActive = true;
        }

        inputActivation[iter->first] = std::max(inputActivation[iter->first], activation);
    }
}

void InputManager::processEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased ||
        event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseButtonReleased ||
        event.type == sf::Event::MouseMoved || event.type == sf::Event::MouseWheelMoved ||
        event.type == sf::Event::MouseWheelScrolled)
    {
        controllerIsActive = false;
    }
}

// Returns whether action is non-zero
bool InputManager::isActionActive(InputAction action)
{
    return (inputActivation[action] != 0.0f);
}

bool InputManager::isActionJustActivated(InputAction action)
{
    if (inputActivationLastFrame[action] == 0.0f)
    {
        return isActionActive(action);
    }

    // Default case
    return false;
}

float InputManager::getActionActivation(InputAction action)
{
    return inputActivation[action];
}

float InputManager::getActionAxisActivation(InputAction negativeAction, InputAction positionAction)
{
    return (inputActivation[positionAction] - inputActivation[negativeAction]);
}

bool InputManager::isControllerActive()
{
    return controllerIsActive;
}