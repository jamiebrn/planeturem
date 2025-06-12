#include "GUI/LandmarkSetGUI.hpp"

void LandmarkSetGUI::initialise(ObjectReference landmarkObject, pl::Color colorA, pl::Color colorB)
{
    aColor = colorA;
    bColor = colorB;

    landmarkSettingObjectReference = landmarkObject;

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

    textDrawData.text = "Color A";
    
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
    
    std::vector<float> replaceColorValue = {aColor.r / 255.0f, aColor.g / 255.0f, aColor.b / 255.0f, 1};
    
    colorDrawData.shader->setUniform4fv("replaceValues", replaceColorValue);
    
    TextDraw::drawText(window, textDrawData);
    TextureManager::drawSubTexture(window, colorDrawData);

    yPos += 100;

    if (guiContext.createColorWheel(scaledPanelPaddingX + panelWidth * intScale / 2, yPos * intScale, 50, aColorValueHSV, aColor).isActive())
    {
        setGUIEvent.modified = true;
    }

    yPos += 80;

    if (guiContext.createSlider(scaledPanelPaddingX + panelWidth / 8 * intScale, yPos * intScale, panelWidth * intScale / 2, 75 * intScale,
        0.0f, 1.0f, &aColorValueHSV, 20 * intScale, "", panelWidth / 8 * intScale, panelWidth / 8 * intScale, 40 * intScale).isHeld())
    {
        pl::Color hsvColor = Helper::convertRGBtoHSV(aColor);
        aColor = Helper::convertHSVtoRGB(hsvColor.r, hsvColor.g, aColorValueHSV);
        setGUIEvent.modified = true;
    }

    yPos += 100;

    textDrawData.position.y = yPos * intScale;
    textDrawData.text = "Color B";

    TextDraw::drawText(window, textDrawData);

    colorDrawData.position = pl::Vector2f(textDrawData.position.x + 36 * 4 * intScale, textDrawData.position.y);
    
    replaceColorValue = {bColor.r / 255.0f, bColor.g / 255.0f, bColor.b / 255.0f, 1};
    colorDrawData.shader->setUniform4fv("replaceValues", replaceColorValue);

    TextureManager::drawSubTexture(window, colorDrawData);

    yPos += 100;

    if (guiContext.createColorWheel(scaledPanelPaddingX + panelWidth * intScale / 2, yPos * intScale, 50, bColorValueHSV, bColor).isActive())
    {
        setGUIEvent.modified = true;
    }

    yPos += 80;

    if (guiContext.createSlider(scaledPanelPaddingX + panelWidth / 8 * intScale, yPos * intScale, panelWidth * intScale / 2, 75 * intScale,
        0.0f, 1.0f, &bColorValueHSV, 20 * intScale, "", panelWidth / 8 * intScale, panelWidth / 8 * intScale, 40 * intScale).isHeld())
    {
        pl::Color hsvColor = Helper::convertRGBtoHSV(bColor);
        bColor = Helper::convertHSVtoRGB(hsvColor.r, hsvColor.g, bColorValueHSV);
        setGUIEvent.modified = true;
    }

    yPos += 100;

    if (guiContext.createButton(scaledPanelPaddingX, yPos * intScale, panelWidth * intScale, 75 * intScale, 24 * intScale, "Set Color", buttonStyle)
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
    return aColor;
}

pl::Color LandmarkSetGUI::getColorB() const
{
    return bColor;
}

const ObjectReference& LandmarkSetGUI::getLandmarkObjectReference() const
{
    return landmarkSettingObjectReference;
}