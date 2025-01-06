#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Event.hpp>

#include <vector>
#include <unordered_map>
#include <optional>

enum class InputAction
{
    UI_UP,
    UI_DOWN,
    UI_LEFT,
    UI_RIGHT,
    UI_CONFIRM,
    UI_CONFIRM_OTHER,
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

    ZOOM_IN,
    ZOOM_OUT,

    PAUSE_GAME,

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

enum class MouseWheelScroll
{
    Up,
    Down
};

struct JoystickAxisWithDirection
{
    sf::Joystick::Axis axis;
    JoystickAxisDirection direction;

    bool operator==(const JoystickAxisWithDirection& other)
    {
        return (axis == other.axis && direction == other.direction);
    }
};

class InputManager
{
    InputManager() = delete;

public:
    static void bindKey(InputAction action, std::optional<sf::Keyboard::Key> key);
    static void bindMouseButton(InputAction action, std::optional<sf::Mouse::Button> button);
    static void bindMouseWheel(InputAction action, std::optional<MouseWheelScroll> wheelDirection);
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
    static float getActionAxisImmediateActivation(InputAction negativeAction, InputAction positionAction);

    static bool isControllerActive();

    // Consumes input for bindings of given action
    // Disables other actions with same key bindings
    static void consumeInputAction(InputAction action);

private:
    template <typename InputType>
    static void bindInput(InputAction action, std::optional<InputType> input, std::unordered_map<InputAction, InputType>& bindMap);

    template<typename InputType>
    static void consumeInput(InputAction action, std::unordered_map<InputAction, InputType>& bindMap);

private:
    static float controllerAxisDeadzone;
    static bool controllerIsActive;

    static std::unordered_map<InputAction, sf::Keyboard::Key> keyBindings;
    static std::unordered_map<InputAction, sf::Mouse::Button> mouseBindings;
    static std::unordered_map<InputAction, MouseWheelScroll> mouseWheelBindings;
    static std::unordered_map<InputAction, JoystickAxisWithDirection> controllerAxisBindings;
    static std::unordered_map<InputAction, unsigned int> controllerButtonBindings;

    static std::unordered_map<InputAction, float> inputActivation;
    static std::unordered_map<InputAction, float> inputActivationLastFrame;

};