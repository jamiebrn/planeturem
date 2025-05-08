#include "GUI/ChatGUI.hpp"
#include "Network/NetworkHandler.hpp"

void ChatGUI::initialise()
{
    chatLog.clear();
    messageBuffer.clear();
    showing = false;
    active = false;
}

void ChatGUI::setShowing(bool enabled)
{
    showing = enabled;
}

void ChatGUI::activate(const pl::RenderTarget& window)
{
    showing = true;
    active = true;

    if (InputManager::isControllerActive())
    {
        float intScale = ResolutionHandler::getResolutionIntegerScale();

        const int width = 600 * intScale;
        const int height = 450 * intScale;
        const int padding = 30 * intScale;

        SteamUtils()->ShowFloatingGamepadTextInput(EFloatingGamepadTextInputMode::k_EFloatingGamepadTextInputModeModeSingleLine,
            window.getWidth() - width - padding, window.getHeight() - height - padding, width, height);
    }
}

bool ChatGUI::isActive()
{
    return active;
}

void ChatGUI::handleEvent(const SDL_Event& event, NetworkHandler& networkHandler)
{
    if (!active)
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

void ChatGUI::update(const pl::RenderTarget& window)
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
        if (active)
        {
            showing = false;
            active = false;
        }

        InputManager::consumeInputAction(InputAction::UI_BACK);
    }
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

    Packet packet;
    packet.set(packetData);

    if (networkHandler.getIsLobbyHost())
    {
        networkHandler.sendPacketToClients(packet, k_nSteamNetworkingSend_Reliable, 0);

        addChatMessage(networkHandler, packetData);
    }
    else
    {
        networkHandler.sendPacketToHost(packet, k_nSteamNetworkingSend_Reliable, 0);
    }

    messageBuffer.clear();
}

void ChatGUI::addChatMessage(NetworkHandler& networkHandler, const PacketDataChatMessage& chatMessagePacket)
{
    if (!networkHandler.isMultiplayerGame())
    {
        return;
    }

    ChatMessage chatMessage;
    chatMessage.userId = chatMessagePacket.userId;

    if (chatMessagePacket.userId == SteamUser()->GetSteamID().ConvertToUint64())
    {
        chatMessage.color = pl::Color(247, 150, 23);
        chatMessage.message = "You: " + chatMessagePacket.message;
    }
    else
    {
        chatMessage.message = networkHandler.getPlayerName(chatMessagePacket.userId) + ": " + chatMessagePacket.message;
    }

    chatLog.push_back(chatMessage);
}

void ChatGUI::draw(pl::RenderTarget& window)
{
    if (!showing)
    {
        return;
    }

    float intScale = ResolutionHandler::getResolutionIntegerScale();

    const int width = 600 * intScale;
    const int height = 450 * intScale;
    const int padding = 30 * intScale;

    const int messageCount = 6;

    pl::VertexArray background;
    background.addQuad(pl::Rect<float>(window.getWidth() - width - padding, window.getHeight() - height - padding, width, height),
        pl::Color(30, 30, 30, 180), pl::Rect<float>());

    window.draw(background, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    pl::TextDrawData drawData;
    drawData.size = 20 * intScale;
    drawData.position = pl::Vector2f(window.getWidth() - padding - width + 10 * intScale, window.getHeight() - padding - 30 * intScale);
    drawData.color = pl::Color(200, 200, 200);

    drawData.text = messageBuffer.empty() ? "Enter a message" : messageBuffer;

    TextDraw::drawText(window, drawData);

    drawData.color = pl::Color(255, 255, 255);

    for (int i = static_cast<int>(chatLog.size()) - 1; i >= std::max(static_cast<int>(chatLog.size()) - messageCount, 0); i--)
    {
        drawData.text = chatLog[i].message;
        drawData.color = chatLog[i].color;
        drawData.position.y -= 30 * intScale;

        TextDraw::drawText(window, drawData);
    }
}