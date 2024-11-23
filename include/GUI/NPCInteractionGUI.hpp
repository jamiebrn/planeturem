#pragma once

#include <optional>

#include <SFML/Graphics.hpp>

#include "Core/TextureManager.hpp"
#include "Core/SpriteBatch.hpp"

#include "GUI/Base/GUIContext.hpp"
#include "GUI/DefaultGUIPanel.hpp"

#include "Object/NPCObject.hpp"

enum class NPCInteractionGUIEventType
{
    Shop,
    Exit
};

struct NPCInteractionGUIEvent
{
    NPCInteractionGUIEventType type;
};

class NPCInteractionGUI : public DefaultGUIPanel
{
public:
    NPCInteractionGUI() = default;

    void initialise(const NPCObject& npcObject);

    void close();

    std::optional<NPCInteractionGUIEvent> createAndDraw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime);

private:
    void drawDialogueBox(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime);

private:
    const NPCObjectData* currentNPCObjectData = nullptr;
    int currentDiagloueIndex = 0;

    std::string dialogueBoxText;
    int dialogueCharIndex = 0;
    float dialogueCharTimer = 0.0f;
    static constexpr float MAX_DIALOGUE_CHAR_TIMER = 0.05f;

};