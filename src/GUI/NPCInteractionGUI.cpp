#include "GUI/NPCInteractionGUI.hpp"

void NPCInteractionGUI::initialise(const NPCObject& npcObject)
{
    currentNPCObjectData = &npcObject.getNPCObjectData();
}

void NPCInteractionGUI::close()
{
    currentNPCObjectData = nullptr;
}

std::optional<NPCInteractionGUIEvent> NPCInteractionGUI::createAndDraw(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

    drawPanel(window);

    const int startElementYPos = resolution.y * 0.37f;
    int elementYPos = startElementYPos;

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    std::optional<NPCInteractionGUIEvent> npcInteractionGUIEvent = std::nullopt;

    if (currentNPCObjectData != nullptr)
    {
        switch (currentNPCObjectData->behaviour)
        {
            case NPCObjectBehaviour::Talk:
            {
                if (guiContext.createButton(scaledPanelPaddingX * intScale, elementYPos, panelWidth * intScale, 75 * intScale, "Talk", buttonStyle).isClicked())
                {
                    // talk
                }

                elementYPos += 100 * intScale;
                break;
            }
            case NPCObjectBehaviour::Shop:
            {
                if (guiContext.createButton(scaledPanelPaddingX * intScale, elementYPos, panelWidth * intScale, 75 * intScale, "Shop", buttonStyle).isClicked())
                {
                    npcInteractionGUIEvent = NPCInteractionGUIEvent();
                    npcInteractionGUIEvent->type = NPCInteractionGUIEventType::Shop;
                }

                elementYPos += 100 * intScale;
                break;
            }
        }
    }

    if (guiContext.createButton(scaledPanelPaddingX * intScale, elementYPos, panelWidth * intScale, 75 * intScale, "Exit", buttonStyle).isClicked())
    {
        npcInteractionGUIEvent = NPCInteractionGUIEvent();
        npcInteractionGUIEvent->type = NPCInteractionGUIEventType::Exit;
    }

    updateAndDrawSelectionHoverRect(window, dt);

    guiContext.draw(window);

    guiContext.endGUI();

    return npcInteractionGUIEvent;
}