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
        // Draw name
        TextDrawData nameTextDrawData;
        nameTextDrawData.text = currentNPCObjectData->npcName;
        nameTextDrawData.position = sf::Vector2f(resolution.x / 2.0f, resolution.y / 2.0f - 22 * 3 * intScale);
        nameTextDrawData.centeredX = true;
        nameTextDrawData.centeredY = true;
        nameTextDrawData.colour = sf::Color(255, 255, 255);
        nameTextDrawData.size = 32 * intScale;

        TextDraw::drawText(window, nameTextDrawData);

        // Draw portrait
        TextureDrawData portraitTextureDrawData;
        portraitTextureDrawData.type = TextureType::Portraits;
        portraitTextureDrawData.position = resolution / 2.0f;
        portraitTextureDrawData.centerRatio = sf::Vector2f(0.5f, 0.5f);
        portraitTextureDrawData.scale = sf::Vector2f(3, 3) * intScale;

        spriteBatch.draw(window, portraitTextureDrawData, sf::IntRect(currentNPCObjectData->portraitTextureOffset, sf::Vector2i(32, 32)));

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