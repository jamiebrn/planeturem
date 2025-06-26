#include "Core/InputManager.hpp"

const Uint8* InputManager::sdlKeyboardState = nullptr;
std::vector<SDL_GameController*> InputManager::sdlGameControllers;

float InputManager::controllerAxisDeadzone = 0.0f;
float InputManager::controllerMouseSens = 600.0f;
bool InputManager::controllerMovedMouseThisFrame = false;
int InputManager::controllerMousePosX = 0;
int InputManager::controllerMousePosY = 0;
bool InputManager::controllerIsActive = false;

int InputManager::controllerGlyphType = 0;

std::unordered_map<InputAction, SDL_Scancode> InputManager::keyBindings;
std::unordered_map<InputAction, int> InputManager::mouseBindings;
std::unordered_map<InputAction, MouseWheelScroll> InputManager::mouseWheelBindings;
std::unordered_map<InputAction, JoystickAxisWithDirection> InputManager::controllerAxisBindings;
std::unordered_map<InputAction, SDL_GameControllerButton> InputManager::controllerButtonBindings;

std::unordered_map<InputAction, float> InputManager::inputActivation;
std::unordered_map<InputAction, float> InputManager::inputActivationLastFrame;

void InputManager::initialise(SDL_Window* window)
{
    sdlKeyboardState = SDL_GetKeyboardState(NULL);

    recentreControllerCursor(window);

    openGameControllers();
}

void InputManager::openGameControllers()
{
    // Close any open game controllers
    for (SDL_GameController* controller : sdlGameControllers)
    {
        SDL_GameControllerClose(controller);
    }

    sdlGameControllers.clear();

    // Open all valid game controllers
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        if (SDL_IsGameController(i))
        {
            sdlGameControllers.push_back(SDL_GameControllerOpen(i));
        }
    }
}

template <typename InputType>
void InputManager::bindInput(InputAction action, std::optional<InputType> input, std::unordered_map<InputAction, InputType>& bindMap, bool overwrite)
{
    if (!input.has_value())
    {
        if (bindMap.contains(action))
        {
            bindMap.erase(action);
        }
        return;
    }

    // Do not overwrite previous binding if overwrite is disabled
    if (bindMap.contains(action) && !overwrite)
    {
        return;
    }
    
    bindMap[action] = input.value();
}

void InputManager::bindKey(InputAction action, std::optional<SDL_Scancode> key, bool overwrite)
{
    bindInput<SDL_Scancode>(action, key, keyBindings, overwrite);
}

void InputManager::bindMouseButton(InputAction action, std::optional<int> button, bool overwrite)
{
    bindInput<int>(action, button, mouseBindings, overwrite);
}

void InputManager::bindMouseWheel(InputAction action, std::optional<MouseWheelScroll> wheelDirection, bool overwrite)
{
    bindInput<MouseWheelScroll>(action, wheelDirection, mouseWheelBindings, overwrite);
}

void InputManager::bindControllerAxis(InputAction action, std::optional<JoystickAxisWithDirection> axisWithDirection, bool overwrite)
{
    bindInput<JoystickAxisWithDirection>(action, axisWithDirection, controllerAxisBindings, overwrite);
}

void InputManager::bindControllerButton(InputAction action, std::optional<SDL_GameControllerButton> button, bool overwrite)
{
    bindInput<SDL_GameControllerButton>(action, button, controllerButtonBindings, overwrite);
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
        if (sdlKeyboardState[iter->second])
        {
            activation = 1.0f;
            controllerIsActive = false;
        }

        inputActivation[iter->first] = std::max(inputActivation[iter->first], activation);
    }

    // Check mouse bindings
    Uint32 mouseButtonState = SDL_GetMouseState(NULL, NULL);

    for (auto iter = mouseBindings.begin(); iter != mouseBindings.end(); iter++)
    {
        float activation = 0.0f;
        if (mouseButtonState & SDL_BUTTON(iter->second))
        {
            activation = 1.0f;
            controllerIsActive = false;
        }

        inputActivation[iter->first] = std::max(inputActivation[iter->first], activation);
    }

    controllerMovedMouseThisFrame = false;

    // Check input from all controllers
    for (SDL_GameController* controller : sdlGameControllers)
    {
        // Check controller axis bindings
        for (auto iter = controllerAxisBindings.begin(); iter != controllerAxisBindings.end(); iter++)
        {
            float activation = 0.0f;
            // float axisPosition = sf::Joystick::getAxisPosition(controllerId, iter->second.axis) / 100.0f;
            float axisPosition = (SDL_GameControllerGetAxis(controller, iter->second.axis) + 0.5f) / 32767.5f;

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
            if (SDL_GameControllerGetButton(controller, iter->second))
            {
                activation = 1.0f;
                controllerIsActive = true;
            }

            inputActivation[iter->first] = std::max(inputActivation[iter->first], activation);
        }
    }

}

void InputManager::processEvent(const SDL_Event& event)
{
    if (event.type == SDL_EventType::SDL_KEYDOWN || event.type == SDL_EventType::SDL_KEYUP ||
        event.type == SDL_EventType::SDL_MOUSEBUTTONDOWN || event.type == SDL_EventType::SDL_MOUSEBUTTONUP ||
        event.type == SDL_EventType::SDL_MOUSEWHEEL || event.type == SDL_EventType::SDL_MOUSEMOTION)
    {
        controllerIsActive = false;
    }

    if (event.type == SDL_EventType::SDL_MOUSEWHEEL)
    {
        MouseWheelScroll wheelDirection = (event.wheel.y < 0) ? MouseWheelScroll::Up : MouseWheelScroll::Down;

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

    if (event.type == SDL_EventType::SDL_CONTROLLERDEVICEADDED ||
        event.type == SDL_EventType::SDL_CONTROLLERDEVICEREMOVED)
    {
        openGameControllers();
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

bool InputManager::isControllerMovingMouse()
{
    return controllerMovedMouseThisFrame;
}

void InputManager::consumeInputAction(InputAction action)
{
    consumeInput<SDL_Scancode>(action, keyBindings);
    consumeInput<int>(action, mouseBindings);
    consumeInput<MouseWheelScroll>(action, mouseWheelBindings);
    consumeInput<JoystickAxisWithDirection>(action, controllerAxisBindings);
    consumeInput<SDL_GameControllerButton>(action, controllerButtonBindings);
}

template <typename InputType>
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

pl::Vector2f InputManager::getMousePosition(SDL_Window* window, float dt)
{
    int mouseX = 0;
    int mouseY = 0;

    SDL_GetMouseState(&mouseX, &mouseY);

    if (InputManager::isControllerActive())
    {
        if (!controllerMovedMouseThisFrame)
        {
            controllerMovedMouseThisFrame = true;

            pl::Vector2f controllerMouseMovement;
            controllerMouseMovement.x = getActionAxisActivation(InputAction::DIRECT_LEFT, InputAction::DIRECT_RIGHT);
            controllerMouseMovement.y = getActionAxisActivation(InputAction::DIRECT_UP, InputAction::DIRECT_DOWN);
            controllerMouseMovement = controllerMouseMovement * controllerMouseSens * dt;

            controllerMousePosX += controllerMouseMovement.x;
            controllerMousePosY += controllerMouseMovement.y;

            int windowWidth = 0;
            int windowHeight = 0;
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);
            
            controllerMousePosX = std::clamp(controllerMousePosX, 0, windowWidth);
            controllerMousePosY = std::clamp(controllerMousePosY, 0, windowHeight);
        }

        return pl::Vector2f(controllerMousePosX, controllerMousePosY);
    }

    return pl::Vector2f(mouseX, mouseY);
}

void InputManager::recentreControllerCursor(SDL_Window* window)
{
    int windowW = 0;
    int windowH = 0;

    SDL_GetWindowSize(window, &windowW, &windowH);

    controllerMousePosX = windowW / 2;
    controllerMousePosY = windowH / 2;
}

std::optional<ControllerGlyph> InputManager::getBoundActionControllerGlyph(InputAction action)
{
    if (!controllerButtonBindings.contains(action))
    {
        if (!controllerAxisBindings.contains(action))
        {
            return std::nullopt;
        }
        else
        {
            if (controllerAxisBindings.at(action).axis == SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT)
            {
                return ControllerGlyph::LEFTTRIGGER;
            }
            else if (controllerAxisBindings.at(action).axis == SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
            {
                return ControllerGlyph::RIGHTTRIGGER;
            }
            else
            {
                return std::nullopt;
            }
        }
    }

    static const std::unordered_map<SDL_GameControllerButton, ControllerGlyph> buttonToGlyph = {
        {SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A, ControllerGlyph::BUTTON_A},
        {SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B, ControllerGlyph::BUTTON_B},
        {SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X, ControllerGlyph::BUTTON_X},
        {SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y, ControllerGlyph::BUTTON_Y},
        {SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, ControllerGlyph::RIGHTSHOULDER},
        {SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER, ControllerGlyph::LEFTSHOULDER},
        {SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK, ControllerGlyph::RIGHTSTICK},
        {SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK, ControllerGlyph::LEFTSTICK},
        {SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_GUIDE, ControllerGlyph::SELECT},
        {SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START, ControllerGlyph::START},
    };

    return buttonToGlyph.at(controllerButtonBindings.at(action));
}

void InputManager::setGlyphType(int type)
{
    controllerGlyphType = (type % CONTROLLER_MAX_GLYPH_TYPE_COUNT + CONTROLLER_MAX_GLYPH_TYPE_COUNT) % CONTROLLER_MAX_GLYPH_TYPE_COUNT;
}

int InputManager::getGlyphType()
{
    return controllerGlyphType;
}

int InputManager::getGlyphTypeCount()
{
    return CONTROLLER_MAX_GLYPH_TYPE_COUNT;
}

void InputManager::loadInputBindingsSave(const InputBindingsSave& bindingsSave)
{
    keyBindings = bindingsSave.keyBindings;
    mouseBindings = bindingsSave.mouseBindings;
    mouseWheelBindings = bindingsSave.mouseWheelBindings;
    controllerAxisBindings = bindingsSave.controllerAxisBindings;
    controllerButtonBindings = bindingsSave.controllerButtonBindings;
    controllerAxisDeadzone = bindingsSave.controllerAxisDeadzone;
}

InputBindingsSave InputManager::createInputBindingsSave()
{
    InputBindingsSave bindingsSave;
    bindingsSave.keyBindings = keyBindings;
    bindingsSave.mouseBindings = mouseBindings;
    bindingsSave.mouseWheelBindings = mouseWheelBindings;
    bindingsSave.controllerAxisBindings = controllerAxisBindings;
    bindingsSave.controllerButtonBindings = controllerButtonBindings;
    bindingsSave.controllerAxisDeadzone = controllerAxisDeadzone;

    return bindingsSave;
}