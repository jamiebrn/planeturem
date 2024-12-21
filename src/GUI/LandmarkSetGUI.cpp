#include "GUI/LandmarkSetGUI.hpp"

void LandmarkSetGUI::initialise(ObjectReference landmarkObject)
{
    aColour[0] = 0.0f;
    aColour[1] = 0.0f;
    aColour[2] = 0.0f;
    bColour[0] = 0.0f;
    bColour[1] = 0.0f;
    bColour[2] = 0.0f;

    landmarkSettingObjectReference = landmarkObject;
}

LandmarkSetGUIEvent LandmarkSetGUI::createAndDraw(sf::RenderWindow& window, float dt)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

    drawPanel(window);

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    LandmarkSetGUIEvent setGUIEvent;

    int yPos = 100;

    TextDrawData textDrawData;
    textDrawData.position = sf::Vector2f(scaledPanelPaddingX + panelWidth / 2 * intScale, yPos * intScale);
    textDrawData.size = 36 * intScale;
    textDrawData.colour = sf::Color(255, 255, 255);
    textDrawData.centeredX = true;
    textDrawData.centeredY = true;

    textDrawData.text = "Colour A";

    TextDraw::drawText(window, textDrawData);

    yPos += 80;

    static const std::array<std::string, 3> colourStrings = {"Red", "Green", "Blue"};

    for (int i = 0; i < 3; i++)
    {
        if (guiContext.createSlider(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale,
            0.0f, 255.0f, &aColour[i], colourStrings[i], panelWidth / 2 * intScale, panelWidth / 10 * intScale, 40 * intScale).isHeld())
        {
            setGUIEvent.modified = true;
        }

        yPos += 100;
    }

    textDrawData.position.y = yPos * intScale;
    textDrawData.text = "Colour B";

    TextDraw::drawText(window, textDrawData);

    yPos += 80;

    for (int i = 0; i < 3; i++)
    {
        if (guiContext.createSlider(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale,
            0.0f, 255.0f, &bColour[i], colourStrings[i], panelWidth / 2 * intScale, panelWidth / 10 * intScale, 40 * intScale).isHeld())
        {
            setGUIEvent.modified = true;
        }

        yPos += 100;
    }

    if (guiContext.createButton(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale, "Set Colour", buttonStyle)
        .isClicked())
    {
        setGUIEvent.closed = true;
    }

    updateAndDrawSelectionHoverRect(window, dt);

    guiContext.draw(window);

    guiContext.endGUI();

    return setGUIEvent;
}

sf::Color LandmarkSetGUI::getColourA() const
{
    return sf::Color(aColour[0], aColour[1], aColour[2]);
}

sf::Color LandmarkSetGUI::getColourB() const
{
    return sf::Color(bColour[0], bColour[1], bColour[2]);
}

const ObjectReference& LandmarkSetGUI::getLandmarkObjectReference() const
{
    return landmarkSettingObjectReference;
}