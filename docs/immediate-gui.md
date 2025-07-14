## Immediate Mode GUI
For Planeturem, I created a simple [immediate mode](https://en.wikipedia.org/wiki/Immediate_mode_(computer_graphics)) GUI system to create the game's menus.

I wanted to use an immediate mode system as a retained mode system seemed a bit disjointed in the fact that UI layout and logical function/drawing were separated.

For example, a retained mode application may have UI defined as follows:

```cpp
// Creation code is separate from logic + draw code, must be ran once before using UI
void createUI()
{
    createButton("Button 1");
    createButton("Button 2");
}

void runUI()
{
    if isButtonPressed("Button 1")
        doSomething();

    if isButtonPressed("Button 2")
        doSomethingElse();

    drawUI();
}
```

Whereas an immediate mode GUI will have an API similar to this:

```cpp
void runUI()
{
    if createButton("Button 1")
        doSomething();

    if createButton("Button 2")
        doSomethingElse();

    drawUI();
}
```

This is easier to work with, but if there is no predefined UI, then how is state tracked across frames? For example, if a button was held last frame, how do we know if it is being held this frame too? As the mouse may no longer be hovering over the UI element, meaning we cannot simply do a bounds check.

This was a question I had before implementing this system. I skimmed [this video](https://www.youtube.com/watch?v=Z1qyvQsjK5Y) for a solution, which came to me assigning IDs based on insertion order in the gui context.
