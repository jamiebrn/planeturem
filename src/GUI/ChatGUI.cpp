#include "GUI/ChatGUI.hpp"
#include "Network/NetworkHandler.hpp"
#include "IO/Log.hpp"

void ChatGUI::initialise()
{
    chatLog.clear();
    messageBuffer.clear();
    showing = false;
    active = false;
    notifyTime = 0.0f;
    menuAlpha = 0.0f;
}

void ChatGUI::setShowing(bool enabled)
{
    showing = enabled;
}

void ChatGUI::activate(const pl::RenderTarget& window)
{
    if (active)
    {
        return;
    }
    
    showing = true;
    active = true;
    notifyTime = 0.0f;

    if (InputManager::isControllerActive())
    {
        float intScale = ResolutionHandler::getResolutionIntegerScale();

        const int width = MENU_WIDTH * intScale;
        int height = MENU_HEIGHT * intScale;
        const int paddingY = MENU_Y_PADDING * intScale;
        const int paddingX = MENU_X_PADDING * intScale;

        SteamUtils()->ShowFloatingGamepadTextInput(EFloatingGamepadTextInputMode::k_EFloatingGamepadTextInputModeModeSingleLine,
            window.getWidth() - width - paddingX, window.getHeight() - height - paddingY, width, height);
    }
}

void ChatGUI::startNotify()
{
    if (!active)
    {
        notifyTime = NOTIFY_TIME_MAX;
        showing = true;
    }
}

bool ChatGUI::isActive()
{
    return active;
}

void ChatGUI::handleEvent(const SDL_Event& event, NetworkHandler& networkHandler)
{
    if (!active || !networkHandler.isMultiplayerGame())
    {
        return;
    }

    if (event.type == SDL_TEXTINPUT)
    {
        messageBuffer += event.text.text;
    }

    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.keysym.scancode == SDL_SCANCODE_RETURN)
        {
            attemptSendMessage(networkHandler);
        }
        if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE && !messageBuffer.empty() && active)
        {
            messageBuffer.pop_back();
        }
    }
}

void ChatGUI::update(const pl::RenderTarget& window, float dt)
{
    if (InputManager::isActionJustActivated(InputAction::OPEN_CHAT))
    {
        if (!active)
        {
            activate(window);
        }

        InputManager::consumeInputAction(InputAction::OPEN_CHAT);
    }
    else if (InputManager::isActionJustActivated(InputAction::UI_BACK))
    {
        if (showing)
        {
            showing = false;
            active = false;
            notifyTime = 0.0f;
        }

        InputManager::consumeInputAction(InputAction::UI_BACK);
    }

    if (notifyTime > 0.0f && !active)
    {
        notifyTime -= dt;
        if (notifyTime <= 0.0f)
        {
            notifyTime = 0.0f;
            showing = false;
        }
    }

    float destMenuAlpha = 0.0f;
    if (showing)
    {
        if (active) destMenuAlpha = 1.0f;
        else destMenuAlpha = MENU_ALPHA_SHOWING;
    }

    menuAlpha = Helper::lerp(menuAlpha, destMenuAlpha, dt * MENU_ALPHA_LERP_WEIGHT);
}

void ChatGUI::attemptSendMessage(NetworkHandler& networkHandler)
{
    if (messageBuffer.empty() || !networkHandler.isMultiplayerGame())
    {
        return;
    }
    
    PacketDataChatMessage packetData;
    packetData.userId = SteamUser()->GetSteamID().ConvertToUint64();
    packetData.message = messageBuffer;

    sendMessageData(networkHandler, packetData);

    messageBuffer.clear();
}

void ChatGUI::sendMessageData(NetworkHandler& networkHandler, const PacketDataChatMessage& chatMessagePacket)
{
    Packet packet;
    packet.set(chatMessagePacket);

    if (networkHandler.isLobbyHostOrSolo())
    {
        networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);

        addChatMessage(networkHandler, chatMessagePacket);
    }
    else
    {
        networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
    }
}

void ChatGUI::addChatMessage(NetworkHandler& networkHandler, const PacketDataChatMessage& chatMessagePacket, bool notify)
{
    ChatMessage chatMessage;
    chatMessage.userId = chatMessagePacket.userId;

    if (!chatMessagePacket.userId.has_value())
    {
        chatMessage.color = pl::Color(232, 59, 59);
        chatMessage.message = " > " + chatMessagePacket.message;
    }
    else if (chatMessagePacket.userId == SteamUser()->GetSteamID().ConvertToUint64())
    {
        chatMessage.color = pl::Color(247, 150, 23);
        chatMessage.message = "You: " + chatMessagePacket.message;
    }
    else
    {
        chatMessage.message = networkHandler.getPlayerName(chatMessagePacket.userId.value()) + ": " + chatMessagePacket.message;
    }

    chatLog.push_back(chatMessage);

    Log::push("CHAT: " + chatMessage.message + "\n");

    if (notify)
    {
        startNotify();
    }
}

void ChatGUI::draw(pl::RenderTarget& window, NetworkHandler& networkHandler, pl::Vector2<uint32_t> playerTile, bool isOnPlanet)
{
    if (menuAlpha <= 0.0f)
    {
        return;
    }

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    const int width = MENU_WIDTH * intScale;
    int height = MENU_HEIGHT * intScale;
    const int paddingY = MENU_Y_PADDING * intScale;
    int paddingX = MENU_Y_PADDING * intScale;
    if (isOnPlanet)
    {
        paddingX = MENU_X_PADDING * intScale;
    }

    const int playerTileTextYSeparation = 15 * intScale;
    const int playerTileTextPadding = 6 * intScale;
    const int playerTileTextYOffset = -9 * intScale;

    // Skip drawing "enter message" prompt if not multiplayer
    if (!networkHandler.isMultiplayerGame())
    {
        height -= 30 * intScale;
    }

    const int messageCount = 10;

    pl::TextDrawData playerTileTextDrawData;
    playerTileTextDrawData.text = "X: " + std::to_string(playerTile.x) + " Y: " + std::to_string(playerTile.y);
    playerTileTextDrawData.size = 24 * intScale;
    playerTileTextDrawData.containPaddingRight = paddingX;
    playerTileTextDrawData.containOnScreenX = true;

    pl::Rect<float> textSize = TextDraw::getTextSize(playerTileTextDrawData);
    playerTileTextDrawData.position.x = window.getWidth() - paddingX - playerTileTextPadding - textSize.width;
    playerTileTextDrawData.position.y = window.getHeight() - height - paddingY - playerTileTextYSeparation - playerTileTextPadding - textSize.height + playerTileTextYOffset;
    playerTileTextDrawData.color.a = menuAlpha * 255.0f;

    pl::VertexArray background;
    background.addQuad(pl::Rect<float>(window.getWidth() - width - paddingX, window.getHeight() - height - paddingY, width, height),
        pl::Color(30, 30, 30, 180 * menuAlpha), pl::Rect<float>());
    
    // Player tile display
    background.addQuad(pl::Rect<float>(window.getWidth() - paddingX - textSize.width - playerTileTextPadding * 2,
        window.getHeight() - height - paddingY - playerTileTextYSeparation - textSize.height - playerTileTextPadding * 2,
        textSize.width + playerTileTextPadding * 2, textSize.height + playerTileTextPadding * 2),
        pl::Color(30, 30, 30, 180 * menuAlpha), pl::Rect<float>());

    window.draw(background, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    // Draw player tile text
    TextDraw::drawText(window, playerTileTextDrawData);

    pl::TextDrawData drawData;
    drawData.size = 20 * intScale;
    
    if (networkHandler.isMultiplayerGame())
    {
        drawData.position = pl::Vector2f(window.getWidth() - paddingX - width + 10 * intScale, window.getHeight() - paddingY - 30 * intScale);
        drawData.color = pl::Color(200, 200, 200, 255 * menuAlpha);
        drawData.text = messageBuffer.empty() ? "Enter a message" : messageBuffer;
        
        TextDraw::drawText(window, drawData);
    }
    else
    {
        drawData.position = pl::Vector2f(window.getWidth() - paddingX - width + 10 * intScale, window.getHeight() - paddingY);
    }

    for (int i = static_cast<int>(chatLog.size()) - 1; i >= std::max(static_cast<int>(chatLog.size()) - messageCount, 0); i--)
    {
        drawData.text = chatLog[i].message;
        drawData.color = chatLog[i].color;
        drawData.color.a = 255 * menuAlpha;
        drawData.position.y -= 30 * intScale;

        TextDraw::drawText(window, drawData);
    }
}