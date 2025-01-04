#include "Core/InputManager.hpp"

float InputManager::controllerAxisDeadzone = 0.0f;
bool InputManager::controllerIsActive = false;

std::unordered_map<InputAction, sf::Keyboard::Key> InputManager::keyBindings;
std::unordered_map<InputAction, sf::Mouse::Button> InputManager::mouseBindings;
std::unordered_map<InputAction, MouseWheelScroll> InputManager::mouseWheelBindings;
std::unordered_map<InputAction, JoystickAxisWithDirection> InputManager::controllerAxisBindings;
std::unordered_map<InputAction, unsigned int> InputManager::controllerButtonBindings;

std::unordered_map<InputAction, float> InputManager::inputActivation;
std::unordered_map<InputAction, float> InputManager::inputActivationLastFrame;

template <typename InputType>
void InputManager::bindInput(InputAction action, std::optional<InputType> input, std::unordered_map<InputAction, InputType>& bindMap)
{
    if (!input.has_value())
    {
        if (bindMap.count(action) > 0)
        {
            bindMap.erase(action);
        }
        return;
    }
    
    bindMap[action] = input.value();
}

void InputManager::bindKey(InputAction action, std::optional<sf::Keyboard::Key> key)
{
    bindInput<sf::Keyboard::Key>(action, key, keyBindings);
}

void InputManager::bindMouseButton(InputAction action, std::optional<sf::Mouse::Button> button)
{
    bindInput<sf::Mouse::Button>(action, button, mouseBindings);
}

void InputManager::bindMouseWheel(InputAction action, std::optional<MouseWheelScroll> wheelDirection)
{
    bindInput<MouseWheelScroll>(action, wheelDirection, mouseWheelBindings);
}

void InputManager::bindControllerAxis(InputAction action, std::optional<JoystickAxisWithDirection> axisWithDirection)
{
    bindInput<JoystickAxisWithDirection>(action, axisWithDirection, controllerAxisBindings);
}

void InputManager::bindControllerButton(InputAction action, std::optional<unsigned int> button)
{
    bindInput<unsigned int>(action, button, controllerButtonBindings);
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

    // Check input from all controllers
    for (int controllerId = 0; sf::Joystick::getIdentification(controllerId).productId != 0; controllerId++)
    {
        // Check controller axis bindings
        for (auto iter = controllerAxisBindings.begin(); iter != controllerAxisBindings.end(); iter++)
        {
            float activation = 0.0f;
            float axisPosition = sf::Joystick::getAxisPosition(controllerId, iter->second.axis) / 100.0f;

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
            if (sf::Joystick::isButtonPressed(controllerId, iter->second))
            {
                activation = 1.0f;
                controllerIsActive = true;
            }

            inputActivation[iter->first] = std::max(inputActivation[iter->first], activation);
        }
    }

}

void InputManager::processEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased ||
        event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseButtonReleased ||
        event.type == sf::Event::MouseMoved || event.type == sf::Event::MouseWheelScrolled)
    {
        controllerIsActive = false;
    }

    if (event.type == sf::Event::MouseWheelScrolled)
    {
        MouseWheelScroll wheelDirection = (event.mouseWheelScroll.delta < 0) ? MouseWheelScroll::Up : MouseWheelScroll::Down;

        for (auto iter = mouseWheelBindings.begin(); iter != mouseWheelBindings.end(); iter++)
        {
            float activation = 0.0f;
            if (iter->second == wheelDirection)
            {
                activation = 1.0f;
                controllerIsActive = false;
            }

            inputActivation[iter->first] = std::max(inputActivation[iter->first], activation);
        }
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

float InputManager::getActionAxisImmediateActivation(InputAction negativeAction, InputAction positionAction)
{
    float negativeImmediateActivation = std::max(inputActivation[negativeAction] - inputActivationLastFrame[negativeAction], 0.0f);
    float positiveImmediateActivation = std::max(inputActivation[positionAction] - inputActivationLastFrame[positionAction], 0.0f);
    return (positiveImmediateActivation - negativeImmediateActivation);
}

bool InputManager::isControllerActive()
{
    return controllerIsActive;
}

void InputManager::consumeInputAction(InputAction action)
{
    consumeInput<sf::Keyboard::Key>(action, keyBindings);
    consumeInput<sf::Mouse::Button>(action, mouseBindings);
    consumeInput<MouseWheelScroll>(action, mouseWheelBindings);
    consumeInput<JoystickAxisWithDirection>(action, controllerAxisBindings);
    consumeInput<unsigned int>(action, controllerButtonBindings);
}

template<typename InputType>
void InputManager::consumeInput(InputAction action, std::unordered_map<InputAction, InputType>& bindMap)
{
    std::vector<InputAction> actionsToConsume;

    if (bindMap.count(action) > 0)
    {
        InputType input = bindMap.at(action);

        // Get other actions with same input
        for (auto iter = bindMap.begin(); iter != bindMap.end(); iter++)
        {
            if (iter->second == input)
            {
                actionsToConsume.push_back(iter->first);
            }
        }
    }

    // Consume inputs
    for (InputAction action : actionsToConsume)
    {
        inputActivationLastFrame[action] = inputActivation[action];
    }
}