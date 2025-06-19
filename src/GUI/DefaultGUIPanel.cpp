#include "GUI/DefaultGUIPanel.hpp"

const ButtonStyle DefaultGUIPanel::buttonStyle = {
    .color = pl::Color(0, 0, 0, 0),
    .hoveredColor = pl::Color(0, 0, 0, 0),
    .clickedColor = pl::Color(0, 0, 0, 0),
    .textColor = pl::Color(200, 200, 200),
    .hoveredTextColor = pl::Color(50, 50, 50),
    .clickedTextColor = pl::Color(255, 255, 255)
};

const SliderStyle DefaultGUIPanel::whiteBlackGradientSliderStyle = {
    .sliderColorLeft = pl::Color(0, 0, 0),
    .sliderColorRight = pl::Color(255, 255, 255),
    .sliderColorLeftHovered = pl::Color(0, 0, 0),
    .sliderColorRightHovered = pl::Color(255, 255, 255),
    .valueRectColor = pl::Color(200, 200, 200),
    .valueRectColorHeld = pl::Color(255, 255, 255)
};

void DefaultGUIPanel::handleEvent(const SDL_Event& event)
{
    guiContext.processEvent(event);
}

void DefaultGUIPanel::drawPanel(pl::RenderTarget& window)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    pl::VertexArray panelRect;

    panelRect.addQuad(pl::Rect<float>(scaledPanelPaddingX, 0, panelWidth * intScale, resolution.y), pl::Color(30, 30, 30, 180), pl::Rect<float>());

    window.draw(panelRect, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);

    updateControllerActivation();
}

void DefaultGUIPanel::updateControllerActivation()
{
    if (deferForceElementActivation)
    {
        deferForceElementActivation = false;
        guiContext.forceElementActivation(selectedElementId);
    }
}

int DefaultGUIPanel::getScaledPanelPaddingX()
{
    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());
    return panelPaddingX * resolution.x / 1920.0f;
}

void DefaultGUIPanel::updateAndDrawSelectionHoverRect(pl::RenderTarget& window, float dt)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    pl::Vector2f resolution = static_cast<pl::Vector2f>(ResolutionHandler::getResolution());

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    if (InputManager::isControllerActive())
    {
        if (!guiContext.isElementActive())
        {
            if (InputManager::isActionJustActivated(InputAction::UI_UP))
            {
                if (selectedElementId == std::numeric_limits<uint64_t>::max())
                {
                    setSelectedElement(0);
                }
                else
                {
                    setSelectedElement(std::clamp(static_cast<int>(selectedElementId) - 1, 0, guiContext.getMaxElementID()));
                }
                guiContext.resetActiveElement();
            }
            if (InputManager::isActionJustActivated(InputAction::UI_DOWN))
            {
                if (selectedElementId == std::numeric_limits<uint64_t>::max())
                {
                    setSelectedElement(0);
                }
                else
                {
                    setSelectedElement(std::clamp(static_cast<int>(selectedElementId + 1), 0, guiContext.getMaxElementID()));
                }
                guiContext.resetActiveElement();
            }
        }
        if (InputManager::isActionJustActivated(InputAction::UI_CONFIRM))
        {
            deferForceElementActivation = true;
        }
        if (InputManager::isActionJustActivated(InputAction::UI_BACK))
        {
            guiContext.resetActiveElement();
            resetHoverRect();
        }
    }
    else
    {
        // If element is active, set as selected
        // Otherwise set hovered element as selected
        if (guiContext.isElementActive())
        {
            setSelectedElement(guiContext.getInputState().activeElement);
        }
        else if (const GUIElement* hoveredElement = guiContext.getHoveredElement();
                hoveredElement != nullptr)
        {
            // updateSelectionHoverRectDestination(hoveredElement->getBoundingBox());
            setSelectedElement(hoveredElement->getElementID());
        }
    }

    CollisionRect panelCollisionRect(scaledPanelPaddingX * intScale, 0, panelWidth * intScale, resolution.y);

    if (deferHoverRectReset || (!panelCollisionRect.isPointInRect(guiContext.getInputState().mouseX, guiContext.getInputState().mouseY) &&
        !guiContext.isElementActive() && !InputManager::isControllerActive()))
    {
        resetHoverRect();
    }

    // Update
    const GUIElement* selectedElement = guiContext.getElementByID(selectedElementId);
    if (selectedElement)
    {
        pl::Rect<float> selectionHoverRectDestination = static_cast<pl::Rect<float>>(selectedElement->getBoundingBox());

        selectionHoverRect.x = Helper::lerp(selectionHoverRect.x, selectionHoverRectDestination.x, 15.0f * dt);
        selectionHoverRect.y = Helper::lerp(selectionHoverRect.y, selectionHoverRectDestination.y, 15.0f * dt);
        selectionHoverRect.width = Helper::lerp(selectionHoverRect.width, selectionHoverRectDestination.width, 15.0f * dt);
        selectionHoverRect.height = Helper::lerp(selectionHoverRect.height, selectionHoverRectDestination.height, 15.0f * dt);
    }
    else
    {
        resetHoverRect();
    }

    pl::VertexArray selectionRect;

    selectionRect.addQuad(selectionHoverRect, pl::Color(200, 200, 200, 150), pl::Rect<float>());

    window.draw(selectionRect, *Shaders::getShader(ShaderType::DefaultNoTexture), nullptr, pl::BlendMode::Alpha);
}

void DefaultGUIPanel::setSelectedElement(ElementID selected)
{
    if (selectedElementId == std::numeric_limits<uint64_t>::max() &&
        selected != std::numeric_limits<uint64_t>::max())
    {
        const GUIElement* selectedElement = guiContext.getElementByID(selected);
        if (selectedElement)
        {
            selectionHoverRect = static_cast<pl::Rect<float>>(selectedElement->getBoundingBox());
        }
    }

    selectedElementId = selected;
}

void DefaultGUIPanel::resetHoverRect()
{
    // selectionHoverRectDestination = pl::Rect<float>(0, 0, 0, 0);
    selectedElementId = std::numeric_limits<uint64_t>::max();
    selectionHoverRect = pl::Rect<float>(0, 0, 0, 0);
    deferHoverRectReset = false;
}

const GUIContext& DefaultGUIPanel::getGUIContext()
{
    return guiContext;
}