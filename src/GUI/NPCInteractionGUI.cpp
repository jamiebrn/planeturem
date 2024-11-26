#include "GUI/NPCInteractionGUI.hpp"

void NPCInteractionGUI::initialise(const NPCObject& npcObject)
{
    currentNPCObjectData = &npcObject.getNPCObjectData();
    currentDiagloueIndex = 0;

    dialogueBoxText = "";
    dialogueBoxCurrentWordBuffer = "";
    dialogueCharIndex = 0;
    dialogueCharTimer = 0.0f;
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
        drawDialogueBox(window, spriteBatch, dt, gameTime);

        // Create talk button by default
        if (guiContext.createButton(scaledPanelPaddingX * intScale, elementYPos, panelWidth * intScale, 75 * intScale, "Talk", buttonStyle).isClicked())
        {
            const std::string& currentDialogue = currentNPCObjectData->dialogueLines.at(currentDiagloueIndex);

            // Skip dialogue animation if playing
            if (dialogueCharIndex < currentDialogue.size())
            {
                // Complete dialogue
                while (!updateDialogue(dt)) {}
            }
            else
            {
                // Advance dialogue
                currentDiagloueIndex = std::min(currentDiagloueIndex + 1, static_cast<int>(currentNPCObjectData->dialogueLines.size()) - 1);
                dialogueBoxText = "";
                dialogueBoxCurrentWordBuffer = "";
                dialogueCharIndex = 0;
                dialogueCharTimer = 0.0f;
            }
        }

        elementYPos += 100 * intScale;

        switch (currentNPCObjectData->behaviour)
        {
            case NPCObjectBehaviour::Shop:
            {
                if (guiContext.createButton(scaledPanelPaddingX * intScale, elementYPos, panelWidth * intScale, 75 * intScale, "Shop", buttonStyle).isClicked())
                {
                    npcInteractionGUIEvent = NPCInteractionGUIEvent();
                    npcInteractionGUIEvent->type = NPCInteractionGUIEventType::Shop;
                    npcInteractionGUIEvent->shopInventoryData = ShopInventoryData(currentNPCObjectData->shopItems, currentNPCObjectData->buyPriceMults,
                        currentNPCObjectData->sellPriceMults);
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

bool NPCInteractionGUI::updateDialogue(float dt)
{
    const std::string& currentDialogue = currentNPCObjectData->dialogueLines.at(currentDiagloueIndex);
    static const int charPerLine = 20;

    if (dialogueCharIndex < currentDialogue.size())
    {
        dialogueCharTimer += dt;
        if (dialogueCharTimer >= MAX_DIALOGUE_CHAR_TIMER)
        {
            dialogueCharTimer = 0.0f;
            char currentChar = currentDialogue.at(dialogueCharIndex);
            dialogueBoxCurrentWordBuffer += currentChar;

            if (currentChar == ' ')
            {
                // Flush buffer and add to dialogue
                dialogueBoxText += dialogueBoxCurrentWordBuffer;
                dialogueBoxCurrentWordBuffer = "";
            }

            dialogueCharIndex++;

            // Make new line
            if (dialogueCharIndex % charPerLine == 0)
            {
                dialogueBoxText += "\n";
            }
        }

        return false;
    }

    // Dialogue completed
    return true;
}

void NPCInteractionGUI::drawDialogueBox(sf::RenderTarget& window, SpriteBatch& spriteBatch, float dt, float gameTime)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

    static const int boxXPadding = 50;
    static const int boxHeight = 200;
    static const int boxWidth = 400;

    int boxXPos = getScaledPanelPaddingX() + (panelWidth + boxXPadding) * intScale;
    int boxYPos = resolution.y / 2.0f - (boxHeight / 2.0f) * intScale;

    // Draw background panel
    sf::RectangleShape backgroundPanel;
    backgroundPanel.setPosition(sf::Vector2f(boxXPos, boxYPos));
    backgroundPanel.setSize(sf::Vector2f(boxWidth, boxHeight) * intScale);
    backgroundPanel.setFillColor(sf::Color(30, 30, 30, 180));

    window.draw(backgroundPanel);

    // Draw name
    TextDrawData nameTextDrawData;
    nameTextDrawData.text = currentNPCObjectData->npcName;
    nameTextDrawData.position = sf::Vector2f(boxXPos + 20 * intScale, boxYPos + 25 * intScale);
    nameTextDrawData.centeredY = true;
    nameTextDrawData.colour = sf::Color(255, 255, 255);
    nameTextDrawData.size = 32 * intScale;

    TextDraw::drawText(window, nameTextDrawData);

    // Draw portrait
    TextureDrawData portraitTextureDrawData;
    portraitTextureDrawData.type = TextureType::Portraits;
    portraitTextureDrawData.position = sf::Vector2f(boxXPos + 20 * intScale, boxYPos + 60 * intScale);
    portraitTextureDrawData.scale = sf::Vector2f(3, 3) * intScale;

    spriteBatch.draw(window, portraitTextureDrawData, sf::IntRect(currentNPCObjectData->portraitTextureOffset, sf::Vector2i(32, 32)));

    // Update dialogue timer
    updateDialogue(dt);

    if (currentDiagloueIndex < currentNPCObjectData->dialogueLines.size())
    {
        // Draw dialogue
        TextDrawData dialogueTextDrawDraw;
        dialogueTextDrawDraw.text = dialogueBoxText + dialogueBoxCurrentWordBuffer;
        dialogueTextDrawDraw.position = sf::Vector2f(boxXPos + 140 * intScale, boxYPos + 90 * intScale);
        dialogueTextDrawDraw.colour = sf::Color(255, 255, 255);
        dialogueTextDrawDraw.size = 24 * intScale;

        TextDraw::drawText(window, dialogueTextDrawDraw);
    }
}