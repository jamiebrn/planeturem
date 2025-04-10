#include "GUI/LandmarkSetGUI.hpp"

void LandmarkSetGUI::initialise(ObjectReference landmarkObject, pl::Color colorA, pl::Color colorB)
{
    aColor[0] = colorA.r;
    aColor[1] = colorA.g;
    aColor[2] = colorA.b;
    bColor[0] = colorB.r;
    bColor[1] = colorB.g;
    bColor[2] = colorB.b;

    landmarkSettingObjectReference = landmarkObject;

    colourPage = 0;

    guiContext.resetActiveElement();
}

LandmarkSetGUIEvent LandmarkSetGUI::createAndDraw(pl::RenderTarget& window, float dt)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());

    drawPanel(window);

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    LandmarkSetGUIEvent setGUIEvent;

    int yPos = 100;

    pl::TextDrawData textDrawData;
    textDrawData.position = pl::Vector2f(scaledPanelPaddingX + panelWidth / 2 * intScale, yPos * intScale);
    textDrawData.size = 36 * intScale;
    textDrawData.color = pl::Color(255, 255, 255);
    textDrawData.centeredX = true;
    textDrawData.centeredY = true;

    textDrawData.text = "Colour A";
    
    static const pl::Rect<int> colorRect(80, 112, 16, 16);

    pl::DrawData colorDrawData;
    colorDrawData.texture = TextureManager::getTexture(TextureType::UI);
    colorDrawData.shader = Shaders::getShader(ShaderType::ReplaceColour);
    colorDrawData.position = pl::Vector2f(textDrawData.position.x + 36 * 4 * intScale, textDrawData.position.y);
    colorDrawData.scale = pl::Vector2f(3, 3) * intScale;
    colorDrawData.centerRatio = pl::Vector2f(0.5f, 0.5f);
    colorDrawData.textureRect = colorRect;
    
    std::vector<float> replaceColorKey = {0, 0, 0, 1};

    colorDrawData.shader->setUniform1i("replaceKeyCount", 1);
    colorDrawData.shader->setUniform4fv("replaceKeys", replaceColorKey);
    
    std::vector<float> replaceColorValue = {aColor[0], aColor[1], aColor[2], 1};
    
    float* colorArray = aColor;
    
    // Change values if on next page
    if (colourPage > 0)
    {
        textDrawData.text = "Colour B";
        replaceColorValue = {bColor[0], bColor[1], bColor[2], 1};
        colorArray = bColor;
    }
    
    colorDrawData.shader->setUniform4fv("replaceValues", replaceColorValue);
    
    TextDraw::drawText(window, textDrawData);
    TextureManager::drawSubTexture(window, colorDrawData);

    yPos += 80;

    static const std::array<std::string, 3> colorStrings = {"Red", "Green", "Blue"};

    for (int i = 0; i < 3; i++)
    {
        if (guiContext.createSlider(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale,
            0.0f, 255.0f, &colorArray[i], 20 * intScale, colorStrings[i], panelWidth / 2 * intScale, panelWidth / 10 * intScale, 40 * intScale).isHeld())
        {
            setGUIEvent.modified = true;
        }

        yPos += 100;
    }

    // Page selection for when window is too small
    if ((yPos + 50 + 80 + 100 * 3 + 75) * intScale >= resolution.y)
    {
        if (guiContext.createButton(scaledPanelPaddingX, yPos * intScale,
            panelWidth / 2 * intScale, 50 * intScale, 20 * intScale, "<", buttonStyle)
            .isClicked())
        {
            colourPage = Helper::wrap(colourPage - 1, 2);
        }

        if (guiContext.createButton((scaledPanelPaddingX + panelWidth / 2) * intScale, yPos * intScale,
            panelWidth / 2 * intScale, 50 * intScale, 20 * intScale, ">", buttonStyle)
            .isClicked())
        {
            colourPage = Helper::wrap(colourPage + 1, 2);
        }

        yPos += 100;
    }
    else
    {
        yPos += 50;
    
        textDrawData.position.y = yPos * intScale;
        textDrawData.text = "Colour B";
    
        TextDraw::drawText(window, textDrawData);
    
        colorDrawData.position = pl::Vector2f(textDrawData.position.x + 36 * 4 * intScale, textDrawData.position.y);
        
        replaceColorValue = {bColor[0], bColor[1], bColor[2], 1};
        colorDrawData.shader->setUniform4fv("replaceValues", replaceColorValue);
    
        TextureManager::drawSubTexture(window, colorDrawData);
    
        yPos += 80;
    
        for (int i = 0; i < 3; i++)
        {
            if (guiContext.createSlider(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale,
                0.0f, 255.0f, &bColor[i], 20 * intScale, colorStrings[i], panelWidth / 2 * intScale, panelWidth / 10 * intScale, 40 * intScale).isHeld())
            {
                setGUIEvent.modified = true;
            }
    
            yPos += 100;
        }
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

pl::Color LandmarkSetGUI::getColorA() const
{
    return pl::Color(aColor[0], aColor[1], aColor[2]);
}

pl::Color LandmarkSetGUI::getColorB() const
{
    return pl::Color(bColor[0], bColor[1], bColor[2]);
}

const ObjectReference& LandmarkSetGUI::getLandmarkObjectReference() const
{
    return landmarkSettingObjectReference;
}