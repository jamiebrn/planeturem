#include "GUI/Base/GUIContext.hpp"

void GUIContext::processEvent(const sf::Event& event)
{
    if (event.type == sf::Event::MouseButtonPressed)
    {
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            inputState.leftMouseJustDown = true;
            inputState.leftMousePressed = true;
        }
        if (event.mouseButton.button == sf::Mouse::Right)
        {
            inputState.rightMouseJustDown = true;
            inputState.rightMousePressed = true;
        }
    }

    if (event.type == sf::Event::MouseButtonReleased)
    {
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            inputState.leftMouseJustUp = true;
            inputState.leftMousePressed = false;

            // activeElementRequiresReset = true;
        }
        if (event.mouseButton.button == sf::Mouse::Right)
        {
            inputState.rightMouseJustUp = true;
            inputState.rightMousePressed = false;
        }
    }

    if (event.type == sf::Event::MouseMoved)
    {
        inputState.mouseX = event.mouseMove.x;
        inputState.mouseY = event.mouseMove.y;
    }

    if (event.type == sf::Event::TextEntered)
    {
        if (event.text.unicode <= 90 && event.text.unicode >= 65 ||
            event.text.unicode >= 97 && event.text.unicode <= 122 ||
            event.text.unicode >= 48 && event.text.unicode <= 57 ||
            event.text.unicode == 95)
        {
            inputState.charEnterBuffer.push_back(event.text.unicode);
        }
    }

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Backspace)
        {
            inputState.backspaceJustPressed = true;
        }
    }
}

void GUIContext::resetActiveElement()
{
    inputState.activeElement = std::numeric_limits<uint64_t>::max();
}

void GUIContext::endGUI()
{
    inputState.leftMouseJustDown = false;
    inputState.leftMouseJustUp = false;

    inputState.rightMouseJustDown = false;
    inputState.rightMouseJustUp = false;

    inputState.charEnterBuffer.clear();
    inputState.backspaceJustPressed = false;

    // if (activeElementRequiresReset)
    // {
    //     resetActiveElement();
    // }
    // activeElementRequiresReset = false;

    elements.clear();
}

const Button& GUIContext::createButton(int x, int y, int width, int height, int textSize, const std::string& text, std::optional<ButtonStyle> style)
{
    std::unique_ptr<Button> button = std::make_unique<Button>(inputState, elements.size(), x, y, width, height, textSize, text, style);

    bool clicked = button->isClicked();

    if (clicked)
    {
        resetActiveElement();
    }
    else if (button->isHeld())
    {
        inputState.activeElement = elements.size();
    }

    elements.push_back(std::move(button));

    return *static_cast<Button*>(elements.back().get());
}

const Checkbox& GUIContext::createCheckbox(int x, int y, int width, int height, int textSize, const std::string& label, bool* value,
    int paddingLeft, int paddingRight, int paddingY)
{
    std::unique_ptr<Checkbox> checkbox = std::make_unique<Checkbox>(inputState, elements.size(), x, y, width, height, textSize, label, value,
        paddingLeft, paddingRight, paddingY);
    
    bool clicked = checkbox->isClicked();

    if (clicked)
    {
        resetActiveElement();
    }
    else if (checkbox->isHeld())
    {
        inputState.activeElement = elements.size();
    }

    elements.push_back(std::move(checkbox));

    return *static_cast<Checkbox*>(elements.back().get());
}

const Slider& GUIContext::createSlider(int x, int y, int width, int height, float minValue, float maxValue, float* value, int textSize, std::optional<std::string> label,
    int paddingLeft, int paddingRight, int paddingY)
{
    std::unique_ptr<Slider> slider = std::make_unique<Slider>(inputState, elements.size(), x, y, width, height, minValue, maxValue, value, textSize, label,
                                                              paddingLeft, paddingRight, paddingY);

    bool held = slider->isHeld();

    if (slider->hasReleased())
    {
        resetActiveElement();
    }
    else if (held)
    {
        inputState.activeElement = elements.size();
    }

    elements.push_back(std::move(slider));

    return *static_cast<Slider*>(elements.back().get());
}

const TextEnter& GUIContext::createTextEnter(int x, int y, int width, int height, int textSize, const std::string& text, std::string* textPtr,
    int paddingX, int paddingY, int maxLength)
{
    std::unique_ptr<TextEnter> textEnter = std::make_unique<TextEnter>(inputState, elements.size(), x, y, width, height, textSize, text, textPtr, paddingX, paddingY, maxLength);

    if (textEnter->hasClickedAway())
    {
        resetActiveElement();
    }
    else if (textEnter->isActive())
    {
        inputState.activeElement = elements.size();
    }

    elements.push_back(std::move(textEnter));

    // return (inputState.activeElement == elements.size() - 1);
    return *static_cast<TextEnter*>(elements.back().get());
}

void GUIContext::draw(sf::RenderTarget& window)
{
    for (auto& guiElement : elements)
    {
        guiElement->draw(window);
    }
}

void GUIContext::forceElementActivation(ElementID element)
{
    inputState.activeElement = element;

    // Simulate press of active element
    inputState.leftMouseJustUp = true;
    inputState.leftMouseJustDown = true;
}

const GUIElement* GUIContext::getHoveredElement() const
{
    for (auto& element : elements)
    {
        if (element->isHovered())
        {
            return element.get();
        }
    }

    return nullptr;
}

const GUIElement* GUIContext::getElementByID(ElementID id) const
{
    if (id < elements.size())
    {
        return elements[id].get();
    }

    return nullptr;
}