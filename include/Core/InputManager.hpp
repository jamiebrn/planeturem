#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Event.hpp>

#include <unordered_map>
#include <optional>

enum class InputAction
{
    UI_UP,
    UI_DOWN,
    UI_LEFT,
    UI_RIGHT,
    UI_CONFIRM,
    UI_BACK,
    UI_TAB_LEFT,
    UI_TAB_RIGHT,

    WALK_UP,
    WALK_DOWN,
    WALK_LEFT,
    WALK_RIGHT,

    USE_TOOL,

    INTERACT,
    OPEN_INVENTORY,

    DIRECT_UP, // Controller specfic
    DIRECT_DOWN,
    DIRECT_LEFT,
    DIRECT_RIGHT
};

enum class JoystickAxisDirection
{
    POSITIVE,
    NEGATIVE
};

struct JoystickAxisWithDirection
{
    sf::Joystick::Axis axis;
    JoystickAxisDirection direction;
};

class InputManager
{
    InputManager() = delete;

public:
    static void bindKey(InputAction action, std::optional<sf::Keyboard::Key> key);
    static void bindMouseButton(InputAction action, std::optional<sf::Mouse::Button> button);
    static void bindControllerAxis(InputAction action, std::optional<JoystickAxisWithDirection> axisWithDirection);
    static void bindControllerButton(InputAction action, std::optional<unsigned int> button);

    static void setControllerAxisDeadzone(float deadzone);

    static void update();

    static void processEvent(const sf::Event& event);

    // Returns whether action is non-zero
    static bool isActionActive(InputAction action);

    static bool isActionJustActivated(InputAction action);

    static float getActionActivation(InputAction action);
    static float getActionAxisActivation(InputAction negativeAction, InputAction positionAction);

    static bool isControllerActive();

private:
    static float controllerAxisDeadzone;
    static bool controllerIsActive;

    static std::unordered_map<InputAction, sf::Keyboard::Key> keyBindings;
    static std::unordered_map<InputAction, sf::Mouse::Button> mouseBindings;
    static std::unordered_map<InputAction, JoystickAxisWithDirection> controllerAxisBindings;
    static std::unordered_map<InputAction, unsigned int> controllerButtonBindings;

    static std::unordered_map<InputAction, float> inputActivation;
    static std::unordered_map<InputAction, float> inputActivationLastFrame;

};