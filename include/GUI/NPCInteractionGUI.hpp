#pragma once

#include <optional>

#include <SFML/Graphics.hpp>

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
    const NPCObjectData* currentNPCObjectData = nullptr;

};