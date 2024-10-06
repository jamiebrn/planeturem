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
    }

    if (event.type == sf::Event::MouseButtonReleased)
    {
        if (event.mouseButton.button == sf::Mouse::Left)
        {
            inputState.leftMouseJustUp = true;
            inputState.leftMousePressed = false;

            // activeElementRequiresReset = true;
        }
    }

    if (event.type == sf::Event::MouseMoved)
    {
        inputState.mouseX = event.mouseMove.x;
        inputState.mouseY = event.mouseMove.y;
    }

    if (event.type == sf::Event::TextEntered)
    {
        bool isChar = ((event.key.scancode >= sf::Keyboard::Scancode::A && event.key.scancode <= sf::Keyboard::Scancode::Num0)
            || event.key.scancode == sf::Keyboard::Scancode::Space);
        if (isChar)
        {
        }
        inputState.charEnterBuffer.push_back(event.text.unicode);
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

    inputState.charEnterBuffer.clear();
    inputState.backspaceJustPressed = false;

    // if (activeElementRequiresReset)
    // {
    //     resetActiveElement();
    // }
    // activeElementRequiresReset = false;

    elements.clear();
}

bool GUIContext::createButton(int x, int y, int width, int height, const std::string& text)
{
    std::unique_ptr<Button> button = std::make_unique<Button>(inputState, elements.size(), x, y, width, height, text);

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

    return clicked;
}

bool GUIContext::createCheckbox(int x, int y, int width, int height, const std::string& label, bool* value)
{
    std::unique_ptr<Checkbox> checkbox = std::make_unique<Checkbox>(inputState, elements.size(), x, y, width, height, label, value);

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

    return clicked;
}

bool GUIContext::createSlider(int x, int y, int width, int height, float minValue, float maxValue, float* value, std::optional<std::string> label)
{
    std::unique_ptr<Slider> slider = std::make_unique<Slider>(inputState, elements.size(), x, y, width, height, minValue, maxValue, value, label);

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

    return held;
}

bool GUIContext::createTextEnter(int x, int y, int width, int height, const std::string& text, std::string* textPtr)
{
    std::unique_ptr<TextEnter> textEnter = std::make_unique<TextEnter>(inputState, elements.size(), x, y, width, height, text, textPtr);

    if (textEnter->hasClickedAway())
    {
        resetActiveElement();
    }
    else if (textEnter->isActive())
    {
        inputState.activeElement = elements.size();
    }

    elements.push_back(std::move(textEnter));

    return (inputState.activeElement == elements.size() - 1);
}

void GUIContext::draw(sf::RenderTarget& window)
{
    for (auto& guiElement : elements)
    {
        guiElement->draw(window);
    }
}