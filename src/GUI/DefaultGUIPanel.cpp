#include "GUI/DefaultGUIPanel.hpp"

const ButtonStyle DefaultGUIPanel::buttonStyle = {
    .colour = sf::Color(0, 0, 0, 0),
    .hoveredColour = sf::Color(0, 0, 0, 0),
    .clickedColour = sf::Color(0, 0, 0, 0),
    .textColour = sf::Color(200, 200, 200),
    .hoveredTextColour = sf::Color(50, 50, 50),
    .clickedTextColour = sf::Color(255, 255, 255)
};

void DefaultGUIPanel::handleEvent(sf::Event& event)
{
    guiContext.processEvent(event);
}

void DefaultGUIPanel::drawPanel(sf::RenderTarget& window)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

    int scaledPanelPaddingX = getScaledPanelPaddingX();

    sf::RectangleShape panelRect(sf::Vector2f(panelWidth * intScale, resolution.y));
    panelRect.setPosition(sf::Vector2f(scaledPanelPaddingX, 0));
    panelRect.setFillColor(sf::Color(30, 30, 30, 180));

    window.draw(panelRect);

    if (deferForceElementActivation)
    {
        deferForceElementActivation = false;
        guiContext.forceElementActivation(selectedElementId);
    }
}

int DefaultGUIPanel::getScaledPanelPaddingX()
{
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());
    return panelPaddingX * resolution.x / 1920.0f;
}

void DefaultGUIPanel::updateAndDrawSelectionHoverRect(sf::RenderTarget& window, float dt)
{
    float intScale = ResolutionHandler::getResolutionIntegerScale();
    sf::Vector2f resolution = static_cast<sf::Vector2f>(ResolutionHandler::getResolution());

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
        sf::FloatRect selectionHoverRectDestination = static_cast<sf::FloatRect>(selectedElement->getBoundingBox());

        selectionHoverRect.left = Helper::lerp(selectionHoverRect.left, selectionHoverRectDestination.left, 15.0f * dt);
        selectionHoverRect.top = Helper::lerp(selectionHoverRect.top, selectionHoverRectDestination.top, 15.0f * dt);
        selectionHoverRect.width = Helper::lerp(selectionHoverRect.width, selectionHoverRectDestination.width, 15.0f * dt);
        selectionHoverRect.height = Helper::lerp(selectionHoverRect.height, selectionHoverRectDestination.height, 15.0f * dt);
    }
    else
    {
        resetHoverRect();
    }

    // Draw
    sf::RectangleShape selectionRect;
    selectionRect.setPosition(sf::Vector2f(selectionHoverRect.left, selectionHoverRect.top));
    selectionRect.setSize(sf::Vector2f(selectionHoverRect.width, selectionHoverRect.height));
    selectionRect.setFillColor(sf::Color(200, 200, 200, 150));

    window.draw(selectionRect);
}

void DefaultGUIPanel::setSelectedElement(ElementID selected)
{
    if (selectedElementId == std::numeric_limits<uint64_t>::max() &&
        selected != std::numeric_limits<uint64_t>::max())
    {
        const GUIElement* selectedElement = guiContext.getElementByID(selected);
        if (selectedElement)
        {
            selectionHoverRect = static_cast<sf::FloatRect>(selectedElement->getBoundingBox());
        }
    }

    selectedElementId = selected;
}

void DefaultGUIPanel::resetHoverRect()
{
    // selectionHoverRectDestination = sf::FloatRect(0, 0, 0, 0);
    selectedElementId = std::numeric_limits<uint64_t>::max();
    selectionHoverRect = sf::FloatRect(0, 0, 0, 0);
    deferHoverRectReset = false;
}

const GUIContext& DefaultGUIPanel::getGUIContext()
{
    return guiContext;
}