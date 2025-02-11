#include "GUI/LandmarkSetGUI.hpp"

void LandmarkSetGUI::initialise(ObjectReference landmarkObject, sf::Color colourA, sf::Color colourB)
{
    aColour[0] = colourA.r;
    aColour[1] = colourA.g;
    aColour[2] = colourA.b;
    bColour[0] = colourB.r;
    bColour[1] = colourB.g;
    bColour[2] = colourB.b;

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

    TextureDrawData colourDrawData;
    colourDrawData.type = TextureType::UI;
    colourDrawData.position = sf::Vector2f(textDrawData.position.x + 36 * 4 * intScale, textDrawData.position.y);
    colourDrawData.scale = sf::Vector2f(3, 3) * intScale;
    colourDrawData.centerRatio = sf::Vector2f(0.5f, 0.5f);

    static const sf::IntRect colourRect(80, 112, 16, 16);

    sf::Glsl::Vec4 replaceColourKey[] = {sf::Glsl::Vec4(sf::Color(0, 0, 0))};
    
    sf::Shader* replaceColourShader = Shaders::getShader(ShaderType::ReplaceColour);
    replaceColourShader->setUniform("replaceKeyCount", 1);
    replaceColourShader->setUniformArray("replaceKeys", replaceColourKey, 1);
    
    sf::Glsl::Vec4 replaceColourValue[] = {sf::Glsl::Vec4(sf::Color(aColour[0], aColour[1], aColour[2]))};
    replaceColourShader->setUniformArray("replaceValues", replaceColourValue, 1);

    TextureManager::drawSubTexture(window, colourDrawData, colourRect, replaceColourShader);

    yPos += 80;

    static const std::array<std::string, 3> colourStrings = {"Red", "Green", "Blue"};

    for (int i = 0; i < 3; i++)
    {
        if (guiContext.createSlider(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale,
            0.0f, 255.0f, &aColour[i], 20 * intScale, colourStrings[i], panelWidth / 2 * intScale, panelWidth / 10 * intScale, 40 * intScale).isHeld())
        {
            setGUIEvent.modified = true;
        }

        yPos += 100;
    }

    yPos += 50;

    textDrawData.position.y = yPos * intScale;
    textDrawData.text = "Colour B";

    TextDraw::drawText(window, textDrawData);

    colourDrawData.position = sf::Vector2f(textDrawData.position.x + 36 * 4 * intScale, textDrawData.position.y);
    
    replaceColourValue[0] = {sf::Glsl::Vec4(sf::Color(bColour[0], bColour[1], bColour[2]))};
    replaceColourShader->setUniformArray("replaceValues", replaceColourValue, 1);

    TextureManager::drawSubTexture(window, colourDrawData, colourRect, replaceColourShader);

    yPos += 80;

    for (int i = 0; i < 3; i++)
    {
        if (guiContext.createSlider(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale,
            0.0f, 255.0f, &bColour[i], 20 * intScale, colourStrings[i], panelWidth / 2 * intScale, panelWidth / 10 * intScale, 40 * intScale).isHeld())
        {
            setGUIEvent.modified = true;
        }

        yPos += 100;
    }

    if (guiContext.createButton(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale, 24 * intScale, "Set Colour", buttonStyle)
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