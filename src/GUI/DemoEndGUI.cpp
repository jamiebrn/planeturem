#include "GUI/DemoEndGUI.hpp"

void DemoEndGUI::initialise()
{
    resetHoverRect();
    guiContext.resetActiveElement();
}

bool DemoEndGUI::createAndDraw(pl::RenderTarget& window, float dt)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());
    
    updateControllerActivation();

    const int backgroundSizeX = 750 * intScale;
    const int backgroundSizeY = 600 * intScale;

    pl::VertexArray backgroundPanel;
    backgroundPanel.addQuad(pl::Rect<float>(resolution.x / 2 - backgroundSizeX / 2, resolution.y / 2 - backgroundSizeY / 2, backgroundSizeX, backgroundSizeY),
        pl::Color(30, 30, 30, 180), pl::Rect<float>());
    window.draw(backgroundPanel, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    pl::TextDrawData textDrawData;
    textDrawData.text = "Thanks for playing!";
    textDrawData.position = pl::Vector2f(resolution.x / 2, resolution.y / 2 - backgroundSizeY / 2 + 10 * intScale);
    textDrawData.size = 36 * intScale;
    textDrawData.color = pl::Color(255, 255, 255);
    textDrawData.centeredX = true;

    TextDraw::drawText(window, textDrawData);

    textDrawData.position.y += 65 * intScale;
    textDrawData.size = 24 * intScale;

    std::vector<std::string> strings = {"You have completed the demo.", "Hope you enjoyed it!", "",
        "I made Planeturem entirely from", "scratch while taking a break", "from education.", "", "If you enjoyed it,", "please consider wishlisting!"};

    for (const std::string& string : strings)
    {
        textDrawData.text = string;
        TextDraw::drawText(window, textDrawData);
        textDrawData.position.y += 25 * intScale;
    }

    textDrawData.position.y += 35 * intScale;
    textDrawData.text = "Features in development";
    textDrawData.size = 32 * intScale;
    TextDraw::drawText(window, textDrawData);

    bool quit = false;

    int yPos = resolution.y / 2 + backgroundSizeY / 2 - 75 * intScale;

    if (guiContext.createButton(resolution.x / 2 - backgroundSizeX / 2, yPos, backgroundSizeX / 2, 75 * intScale, 24 * intScale, "Wishlist", buttonStyle).isClicked())
    {
        SteamFriends()->ActivateGameOverlayToStore(STEAM_APP_ID, EOverlayToStoreFlag::k_EOverlayToStoreFlag_None);
    }

    if (guiContext.createButton(resolution.x / 2, yPos, backgroundSizeX / 2, 75 * intScale, 24 * intScale, "Go Back", buttonStyle).isClicked())
    {
        quit = true;
    }

    updateAndDrawSelectionHoverRect(window, dt);

    guiContext.draw(window);

    guiContext.endGUI();

    return quit;
}