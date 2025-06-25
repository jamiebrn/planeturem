#pragma once

#include <cmath>
#include <vector>
#include <string>
#include <optional>

#include <extlib/steam/steam_api.h>

#include <SDL2/SDL_events.h>

#include <Graphics/VertexArray.hpp>
#include <Graphics/SpriteBatch.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/TextureManager.hpp"
#include "Core/Shaders.hpp"
#include "Core/TextDraw.hpp"
#include "Core/ResolutionHandler.hpp"
#include "Core/InputManager.hpp"

#include "Network/PacketData/PacketDataIncludes.hpp"

class NetworkHandler;

struct ChatMessage
{
    std::optional<uint64_t> userId;
    std::string message;
    pl::Color color;
};

class ChatGUI
{
public:
    void initialise();

    void setShowing(bool enabled);
    void activate(const pl::RenderTarget& window);

    void startNotify();

    bool isActive();

    void handleEvent(const SDL_Event& event, NetworkHandler& networkHandler);

    void update(const pl::RenderTarget& window, float dt);

    void draw(pl::RenderTarget& window, NetworkHandler& networkHandler, pl::Vector2<uint32_t> playerTile, bool isOnPlanet);

    void addChatMessage(NetworkHandler& networkHandler, const PacketDataChatMessage& chatMessagePacket, bool notify = true);

    void sendMessageData(NetworkHandler& networkHandler, const PacketDataChatMessage& chatMessagePacket);

private:
    void attemptSendMessage(NetworkHandler& networkHandler);

    std::vector<ChatMessage> chatLog;

    std::string messageBuffer;

    bool showing = false;
    bool active = false;

    static constexpr float NOTIFY_TIME_MAX = 4.0f;
    float notifyTime;
    static constexpr float MENU_ALPHA_LERP_WEIGHT = 11.0f;
    static constexpr float MENU_ALPHA_SHOWING = 0.5f;
    float menuAlpha;

    static constexpr float MENU_WIDTH = 600.0f;
    static constexpr float MENU_HEIGHT = 335.0f;
    static constexpr float MENU_X_PADDING = 340.0f;
    static constexpr float MENU_Y_PADDING = 30.0f;

};