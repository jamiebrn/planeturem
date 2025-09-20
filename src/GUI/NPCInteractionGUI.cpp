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

std::optional<NPCInteractionGUIEvent> NPCInteractionGUI::createAndDraw(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, float dt, float gameTime)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());

    drawPanel(window);

    const int startElementYPos = resolution.y * 0.37f;
    int elementYPos = startElementYPos;

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    std::optional<NPCInteractionGUIEvent> npcInteractionGUIEvent = std::nullopt;

    if (currentNPCObjectData != nullptr)
    {
        drawDialogueBox(window, spriteBatch, dt, gameTime);

        // Create talk button by default
        if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, 24 * intScale, "Talk", buttonStyle).isClicked())
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

        if (currentNPCObjectData->behaviour == NPCObjectBehaviour::Shop)
        {
            if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, 24 * intScale, "Shop", buttonStyle).isClicked())
            {
                npcInteractionGUIEvent = NPCInteractionGUIEvent();
                npcInteractionGUIEvent->type = NPCInteractionGUIEventType::Shop;
                npcInteractionGUIEvent->shopInventoryData = ShopInventoryData(currentNPCObjectData->shopItems, currentNPCObjectData->buyPriceMults,
                    currentNPCObjectData->sellPriceMults);
            }

            elementYPos += 100 * intScale;
        }
    }

    if (guiContext.createButton(scaledPanelPaddingX, elementYPos, panelWidth * intScale, 75 * intScale, 24 * intScale, "Exit", buttonStyle).isClicked())
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

void NPCInteractionGUI::drawDialogueBox(pl::RenderTarget& window, pl::SpriteBatch& spriteBatch, float dt, float gameTime)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());

    static const int boxXPadding = 50;
    static const int boxHeight = 200;
    static const int boxWidth = 450;

    int boxXPos = getScaledPanelPaddingX() + (panelWidth + boxXPadding) * intScale;
    int boxYPos = resolution.y / 2.0f - (boxHeight / 2.0f) * intScale;

    // Draw background panel
    pl::VertexArray backgroundPanel;

    backgroundPanel.addQuad(pl::Rect<float>(boxXPos, boxYPos, boxWidth, boxHeight), pl::Color(30, 30, 30, 180), pl::Rect<float>());

    window.draw(backgroundPanel, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    // Draw name
    pl::TextDrawData nameTextDrawData;
    nameTextDrawData.text = currentNPCObjectData->npcName;
    nameTextDrawData.position = pl::Vector2f(boxXPos + 20 * intScale, boxYPos + 25 * intScale);
    nameTextDrawData.centeredY = true;
    nameTextDrawData.color = pl::Color(255, 255, 255);
    nameTextDrawData.size = 32 * intScale;

    TextDraw::drawText(window, nameTextDrawData);

    // Draw portrait
    pl::DrawData portraitTextureDrawData;
    portraitTextureDrawData.texture = TextureManager::getTexture(TextureType::Portraits);
    portraitTextureDrawData.shader = Shaders::getShader(ShaderType::Default);
    portraitTextureDrawData.position = pl::Vector2f(boxXPos + 20 * intScale, boxYPos + 60 * intScale);
    portraitTextureDrawData.scale = pl::Vector2f(3, 3) * intScale;
    portraitTextureDrawData.textureRect = pl::Rect<int>(currentNPCObjectData->portraitTextureOffset, pl::Vector2<int>(32, 32));

    spriteBatch.draw(window, portraitTextureDrawData);

    // Update dialogue timer
    updateDialogue(dt);

    if (currentDiagloueIndex < currentNPCObjectData->dialogueLines.size())
    {
        // Draw dialogue
        pl::TextDrawData dialogueTextDrawDraw;
        dialogueTextDrawDraw.text = dialogueBoxText + dialogueBoxCurrentWordBuffer;
        dialogueTextDrawDraw.position = pl::Vector2f(boxXPos + 140 * intScale, boxYPos + 112 * intScale);
        dialogueTextDrawDraw.color = pl::Color(255, 255, 255);
        dialogueTextDrawDraw.size = 24 * intScale;
        dialogueTextDrawDraw.centeredY = true;

        TextDraw::drawText(window, dialogueTextDrawDraw);
    }
}