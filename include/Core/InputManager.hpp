#pragma once

#include <SDL2/SDL_events.h>

#include <Vector.hpp>

#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <optional>

#include <Core/json.hpp>

#define MAGIC_ENUM_RANGE_MAX SDL_NUM_SCANCODES
#include <extlib/magic_enum.hpp>

#include "Core/Helper.hpp"
#include "Core/ResolutionHandler.hpp"

enum class InputAction
{
    UI_UP,
    UI_DOWN,
    UI_LEFT,
    UI_RIGHT,
    UI_CONFIRM,
    UI_CONFIRM_OTHER,
    UI_SHIFT,
    UI_CTRL,
    UI_BACK,
    UI_TAB_LEFT,
    UI_TAB_RIGHT,

    HOTBAR_0,
    HOTBAR_1,
    HOTBAR_2,
    HOTBAR_3,
    HOTBAR_4,
    HOTBAR_5,
    HOTBAR_6,
    HOTBAR_7,

    WALK_UP,
    WALK_DOWN,
    WALK_LEFT,
    WALK_RIGHT,

    USE_TOOL,

    INTERACT,
    OPEN_INVENTORY,
    DROP_ITEM,

    ZOOM_IN,
    ZOOM_OUT,

    OPEN_CHAT,
    OPEN_MAP,

    PAUSE_GAME,

    DIRECT_UP, // Controller specfic
    DIRECT_DOWN,
    DIRECT_LEFT,
    DIRECT_RIGHT,

    TOGGLE_CONTROLLER_AIM_MODE
};

enum class ControllerGlyph
{
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    RIGHTSHOULDER,
    LEFTSHOULDER,
    RIGHTTRIGGER,
    LEFTTRIGGER,
    RIGHTSTICK,
    LEFTSTICK,
    SELECT,
    START
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
    SDL_GameControllerAxis axis = SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_INVALID;
    JoystickAxisDirection direction;

    bool operator==(const JoystickAxisWithDirection& other) const
    {
        return (axis == other.axis && direction == other.direction);
    }
};

inline void from_json(const nlohmann::json& json, JoystickAxisWithDirection& joystick)
{
    auto joystickAxis = magic_enum::enum_cast<SDL_GameControllerAxis>(json["axis"].get<std::string>());
    auto joystickDirection = magic_enum::enum_cast<JoystickAxisDirection>(json["direction"].get<std::string>());

    if (joystickAxis.has_value())
    {
        joystick.axis = joystickAxis.value();
    }
    if (joystickDirection.has_value())
    {
        joystick.direction = joystickDirection.value();
    }
}

inline void to_json(nlohmann::json& json, const JoystickAxisWithDirection& joystick)
{
    json["axis"] = magic_enum::enum_name(joystick.axis);
    json["direction"] = magic_enum::enum_name(joystick.direction);
}

struct InputBindingsSave
{
    std::unordered_map<InputAction, SDL_Scancode> keyBindings;
    std::unordered_map<InputAction, int> mouseBindings;
    std::unordered_map<InputAction, MouseWheelScroll> mouseWheelBindings;
    std::unordered_map<InputAction, JoystickAxisWithDirection> controllerAxisBindings;
    std::unordered_map<InputAction, SDL_GameControllerButton> controllerButtonBindings;

    float controllerAxisDeadzone;

    template <typename T, bool isEnum>
    inline void writeBindings(nlohmann::json& json, const std::unordered_map<InputAction, T>& bindings, const std::string& name) const
    {
        for (const auto& binding : bindings)
        {
            auto inputActionString = magic_enum::enum_name(binding.first);

            if constexpr (isEnum)
            {
                json[name][inputActionString] = magic_enum::enum_name(binding.second);
                continue;
            }

            json[name][inputActionString] = binding.second;
        }
    }

    template <typename T, bool isEnum>
    inline void loadBindings(const nlohmann::json& json, std::unordered_map<InputAction, T>& bindings, const std::string& name)
    {
        if (!json.contains(name))
        {
            return;
        }
        
        for (auto iter = json[name].begin(); iter != json[name].end(); iter++)
        {
            auto inputAction = magic_enum::enum_cast<InputAction>(iter.key());
            if (!inputAction.has_value())
            {
                continue;
            }

            if constexpr (isEnum)
            {
                auto enumCasted = magic_enum::enum_cast<T>(iter.value().get<std::string>());
                if (enumCasted.has_value())
                {
                    bindings[inputAction.value()] = enumCasted.value();
                }
                continue;
            }

            try
            {
                bindings[inputAction.value()] = iter.value().get<T>();
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    }
};

inline void from_json(const nlohmann::json& json, InputBindingsSave& bindingsSave)
{
    bindingsSave.loadBindings<SDL_Scancode, true>(json, bindingsSave.keyBindings, "keys");
    bindingsSave.loadBindings<int, false>(json, bindingsSave.mouseBindings, "mouse-buttons");
    bindingsSave.loadBindings<MouseWheelScroll, true>(json, bindingsSave.mouseWheelBindings, "mouse-wheel");
    bindingsSave.loadBindings<JoystickAxisWithDirection, false>(json, bindingsSave.controllerAxisBindings, "controller-axis");
    bindingsSave.loadBindings<SDL_GameControllerButton, true>(json, bindingsSave.controllerButtonBindings, "controller-button");

    bindingsSave.controllerAxisDeadzone = json["controller-axis-deadzone"];
}

inline void to_json(nlohmann::json& json, const InputBindingsSave& bindingsSave)
{
    bindingsSave.writeBindings<SDL_Scancode, true>(json, bindingsSave.keyBindings, "keys");
    bindingsSave.writeBindings<int, false>(json, bindingsSave.mouseBindings, "mouse-buttons");
    bindingsSave.writeBindings<MouseWheelScroll, true>(json, bindingsSave.mouseWheelBindings, "mouse-wheel");
    bindingsSave.writeBindings<JoystickAxisWithDirection, false>(json, bindingsSave.controllerAxisBindings, "controller-axis");
    bindingsSave.writeBindings<SDL_GameControllerButton, true>(json, bindingsSave.controllerButtonBindings, "controller-button");

    json["controller-axis-deadzone"] = bindingsSave.controllerAxisDeadzone;
}

class InputManager
{
    InputManager() = delete;

public:
    static void initialise(SDL_Window* window);
    
    static void bindKey(InputAction action, std::optional<SDL_Scancode> key, bool overwrite = true);
    static void bindMouseButton(InputAction action, std::optional<int> button, bool overwrite = true);
    static void bindMouseWheel(InputAction action, std::optional<MouseWheelScroll> wheelDirection, bool overwrite = true);
    static void bindControllerAxis(InputAction action, std::optional<JoystickAxisWithDirection> axisWithDirection, bool overwrite = true);
    static void bindControllerButton(InputAction action, std::optional<SDL_GameControllerButton> button, bool overwrite = true);

    static void setControllerAxisDeadzone(float deadzone);

    static void update(SDL_Window* window, float dt, pl::Vector2f playerScreenPos);

    static void processEvent(const SDL_Event& event);

    // Returns whether action is non-zero
    static bool isActionActive(InputAction action);

    static bool isActionJustActivated(InputAction action);

    static float getActionActivation(InputAction action);
    static float getActionAxisActivation(InputAction negativeAction, InputAction positionAction);
    static float getActionAxisImmediateActivation(InputAction negativeAction, InputAction positionAction);

    static bool isControllerActive();
    static bool isControllerMovingMouse();

    // Consumes input for bindings of given action
    // Disables other actions with same key bindings
    static void consumeInputAction(InputAction action);

    static pl::Vector2f getMousePosition(SDL_Window* window);

    static void recentreControllerCursor(SDL_Window* window);

    static std::optional<ControllerGlyph> getBoundActionControllerGlyph(InputAction action);

    static void setGlyphType(int type);
    static int getGlyphType();

    static int getGlyphTypeCount();

    static void loadInputBindingsSave(const InputBindingsSave& bindingsSave);
    static InputBindingsSave createInputBindingsSave();

    static void setControllerRelativeAimMode(SDL_Window* window, bool relativeMode);
    static bool getControllerRelativeAimMode();
    static float getControllerRelativeCursorAlpha();

private:
    template <typename InputType>
    static void bindInput(InputAction action, std::optional<InputType> input, std::unordered_map<InputAction, InputType>& bindMap, bool overwrite);

    template<typename InputType>
    static void consumeInput(InputAction action, std::unordered_map<InputAction, InputType>& bindMap);

    static void openGameControllers();

private:
    static const Uint8* sdlKeyboardState;
    static std::vector<SDL_GameController*> sdlGameControllers;

    static float controllerAxisDeadzone;
    static float controllerMouseSens;
    // static bool controllerMovedMouseThisFrame;
    static int controllerMousePosX;
    static int controllerMousePosY;
    static bool controllerIsActive;

    static bool controllerRelativeAimMode;
    static constexpr float CONTROLLER_RELATIVE_AIM_MODE_LERP_WEIGHT = 7.0f;

    static constexpr int CONTROLLER_MAX_GLYPH_TYPE_COUNT = 4;
    static int controllerGlyphType;

    static std::unordered_map<InputAction, SDL_Scancode> keyBindings;
    static std::unordered_map<InputAction, int> mouseBindings;
    static std::unordered_map<InputAction, MouseWheelScroll> mouseWheelBindings;
    static std::unordered_map<InputAction, JoystickAxisWithDirection> controllerAxisBindings;
    static std::unordered_map<InputAction, SDL_GameControllerButton> controllerButtonBindings;

    static std::unordered_map<InputAction, float> inputActivation;
    static std::unordered_map<InputAction, float> inputActivationLastFrame;

};