#pragma once

#include <SDL2/SDL_events.h>

#include <Vector.hpp>

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <optional>

#include <Core/json.hpp>

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

    ZOOM_IN,
    ZOOM_OUT,

    OPEN_CHAT,

    PAUSE_GAME,

    DIRECT_UP, // Controller specfic
    DIRECT_DOWN,
    DIRECT_LEFT,
    DIRECT_RIGHT,

    RECENTRE_CONTROLLER_CURSOR
};

NLOHMANN_JSON_SERIALIZE_ENUM(InputAction, {
    {InputAction::UI_UP, "UI_UP"},
    {InputAction::UI_DOWN, "UI_DOWN"},
    {InputAction::UI_LEFT, "UI_LEFT"},
    {InputAction::UI_RIGHT, "UI_RIGHT"},
    {InputAction::UI_CONFIRM, "UI_CONFIRM"},
    {InputAction::UI_CONFIRM_OTHER, "UI_CONFIRM_OTHER"},
    {InputAction::UI_SHIFT, "UI_SHIFT"},
    {InputAction::UI_CTRL, "UI_CTRL"},
    {InputAction::UI_BACK, "UI_BACK"},
    {InputAction::UI_TAB_LEFT, "UI_TAB_LEFT"},
    {InputAction::UI_TAB_RIGHT, "UI_TAB_RIGHT"},
    {InputAction::HOTBAR_0, "HOTBAR_0"},
    {InputAction::HOTBAR_1, "HOTBAR_1"},
    {InputAction::HOTBAR_2, "HOTBAR_2"},
    {InputAction::HOTBAR_3, "HOTBAR_3"},
    {InputAction::HOTBAR_4, "HOTBAR_4"},
    {InputAction::HOTBAR_5, "HOTBAR_5"},
    {InputAction::HOTBAR_6, "HOTBAR_6"},
    {InputAction::HOTBAR_7, "HOTBAR_7"},
    {InputAction::WALK_UP, "WALK_UP"},
    {InputAction::WALK_DOWN, "WALK_DOWN"},
    {InputAction::WALK_LEFT, "WALK_LEFT"},
    {InputAction::WALK_RIGHT, "WALK_RIGHT"},
    {InputAction::USE_TOOL, "USE_TOOL"},
    {InputAction::INTERACT, "INTERACT"},
    {InputAction::OPEN_INVENTORY, "OPEN_INVENTORY"},
    {InputAction::ZOOM_IN, "ZOOM_IN"},
    {InputAction::ZOOM_OUT, "ZOOM_OUT"},
    {InputAction::OPEN_CHAT, "OPEN_CHAT"},
    {InputAction::PAUSE_GAME, "PAUSE_GAME"},
    {InputAction::DIRECT_UP, "DIRECT_UP"},
    {InputAction::DIRECT_DOWN, "DIRECT_DOWN"},
    {InputAction::DIRECT_LEFT, "DIRECT_LEFT"},
    {InputAction::DIRECT_RIGHT, "DIRECT_RIGHT"},
    {InputAction::RECENTRE_CONTROLLER_CURSOR, "RECENTRE_CONTROLLER_CURSOR"}
})

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

NLOHMANN_JSON_SERIALIZE_ENUM(JoystickAxisDirection, {JoystickAxisDirection::POSITIVE, "positive", JoystickAxisDirection::NEGATIVE, "negative"})

enum class MouseWheelScroll
{
    Up,
    Down
};

NLOHMANN_JSON_SERIALIZE_ENUM(MouseWheelScroll, {MouseWheelScroll::Up, "up", MouseWheelScroll::Down, "down"})

struct JoystickAxisWithDirection
{
    SDL_GameControllerAxis axis;
    JoystickAxisDirection direction;

    bool operator==(const JoystickAxisWithDirection& other) const
    {
        return (axis == other.axis && direction == other.direction);
    }
    
    inline void from_json(const nlohmann::json& json)
    {
        axis = json["axis"];
        direction = json["direction"];
    }

    inline void to_json(nlohmann::json& json)
    {
        json["axis"] = axis;
        json["direction"] = direction;
    }
};

struct InputBindingsSave
{
    std::unordered_map<InputAction, SDL_Scancode> keyBindings;
    std::unordered_map<InputAction, int> mouseBindings;
    std::unordered_map<InputAction, MouseWheelScroll> mouseWheelBindings;
    std::unordered_map<InputAction, JoystickAxisWithDirection> controllerAxisBindings;
    std::unordered_map<InputAction, SDL_GameControllerButton> controllerButtonBindings;

    float controllerAxisDeadzone;

    template <typename T>
    inline void writeBindings(nlohmann::json& json, const std::unordered_map<InputAction, T>& bindings, const std::string& name) const
    {
        for (const auto& binding : bindings)
        {
            nlohmann::json inputActionStringJson = binding.first;
            std::string inputActionString = inputActionStringJson.get<std::string>();
            json[name][inputActionString] = binding.second;
        }
    }

    template <typename T>
    inline void loadBindings(const nlohmann::json& json, std::unordered_map<InputAction, T>& bindings, const std::string& name)
    {
        for (auto iter = json[name].begin(); iter != json[name].end(); iter++)
        {
            nlohmann::json inputActionStringJson = iter.key();
            InputAction inputAction = inputActionStringJson.get<InputAction>();
            bindings[inputAction] = iter.value().get<T>();
        }
    }
};

inline void from_json(const nlohmann::json& json, InputBindingsSave& bindingsSave)
{
    bindingsSave.loadBindings(json, bindingsSave.keyBindings, "keys");
    bindingsSave.loadBindings(json, bindingsSave.mouseBindings, "mouse-buttons");
    bindingsSave.loadBindings(json, bindingsSave.mouseWheelBindings, "mouse-wheel");
    bindingsSave.loadBindings(json, bindingsSave.controllerAxisBindings, "controller-axis");
    bindingsSave.loadBindings(json, bindingsSave.controllerButtonBindings, "controller-button");

    bindingsSave.controllerAxisDeadzone = json["controller-axis-deadzone"];
}

inline void to_json(nlohmann::json& json, const InputBindingsSave& bindingsSave)
{
    bindingsSave.writeBindings(json, bindingsSave.keyBindings, "keys");
    bindingsSave.writeBindings(json, bindingsSave.mouseBindings, "mouse-buttons");
    bindingsSave.writeBindings(json, bindingsSave.mouseWheelBindings, "mouse-wheel");
    bindingsSave.writeBindings(json, bindingsSave.controllerAxisBindings, "controller-axis");
    bindingsSave.writeBindings(json, bindingsSave.controllerButtonBindings, "controller-button");

    json["controller-axis-deadzone"] = bindingsSave.controllerAxisDeadzone;
}

class InputManager
{
    InputManager() = delete;

public:
    static void initialise(SDL_Window* window);

    static void bindKey(InputAction action, std::optional<SDL_Scancode> key);
    static void bindMouseButton(InputAction action, std::optional<int> button);
    static void bindMouseWheel(InputAction action, std::optional<MouseWheelScroll> wheelDirection);
    static void bindControllerAxis(InputAction action, std::optional<JoystickAxisWithDirection> axisWithDirection);
    static void bindControllerButton(InputAction action, std::optional<SDL_GameControllerButton> button);

    static void setControllerAxisDeadzone(float deadzone);

    static void update();

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

    static pl::Vector2f getMousePosition(SDL_Window* window, float dt);

    static void recentreControllerCursor(SDL_Window* window);

    static std::optional<ControllerGlyph> getBoundActionControllerGlyph(InputAction action);

    static void setGlyphType(int type);
    static int getGlyphType();

    static int getGlyphTypeCount();

    static void loadInputBindingsSave(const InputBindingsSave& bindingsSave);
    static InputBindingsSave createInputBindingsSave();

private:
    template <typename InputType>
    static void bindInput(InputAction action, std::optional<InputType> input, std::unordered_map<InputAction, InputType>& bindMap);

    template<typename InputType>
    static void consumeInput(InputAction action, std::unordered_map<InputAction, InputType>& bindMap);

    static void openGameControllers();

private:
    static const Uint8* sdlKeyboardState;
    static std::vector<SDL_GameController*> sdlGameControllers;

    static float controllerAxisDeadzone;
    static float controllerMouseSens;
    static bool controllerMovedMouseThisFrame;
    static int controllerMousePosX;
    static int controllerMousePosY;
    static bool controllerIsActive;

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