## Immediate Mode GUI
For Planeturem, I created a simple [immediate mode](https://en.wikipedia.org/wiki/Immediate_mode_(computer_graphics)) GUI system to create the game's menus.

I wanted to use an immediate mode system as a retained mode system seemed a bit disjointed due to UI layout and logical function/drawing being separate.

For example, a retained mode application may have UI defined as follows:

```cpp
// Creation code is separate from logic + draw code, must be run once before using UI
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

This was a question I had before implementing this system. I skimmed [this video](https://www.youtube.com/watch?v=Z1qyvQsjK5Y) for a solution, which came to me assigning IDs based on insertion order in the GUI context. This allowed tracking of state across frames, but meant a change in UI definition could cause the incorrect element to be marked as active (I will address this later).

### GUI Context
The `GUIContext` class handles a set of GUI elements, and draws them when finished defining the UI. It tracks active element IDs and input states (mouse clicks, etc).

A vector of `std::unique_ptr<GUIElement>` is used to store all elements, with `GUIElement` being the abstract base class/interface. Each GUI element type has a constructor function in the GUI context, e.g. `createButton(...)`, which will construct the element and place it in the vector, "activating" it as the element's ID is the currently active ID.

### GUI Element
`GUIElement` provides an interface for all GUI elements. It is defined as follows:
```cpp
class GUIElement
{
public:
    inline GUIElement(ElementID id, int textSize) : id(id), textSize(textSize) {}
    
    virtual void draw(pl::RenderTarget& window) = 0;

    virtual pl::Rect<int> getBoundingBox() const = 0;

    inline bool isHovered() const {return hovered;} 

    inline ElementID getElementID() const {return id;}

protected:
    ElementID id;

    int textSize = 0;

    bool hovered = false;

};
```
This provides basic functionality, where the `draw` and `getBoundingBox` functions need to be implemented for each GUI element, as they will each behave differently.

Each GUI element also has a unique constructor as they are different elements, so require different data to be constructed. For example, this is the constructor for the `Button` element:
```cpp
Button(const GUIInputState& inputState, ElementID id, int x, int y, int width, int height, int textSize, const std::string& text,
    std::optional<ButtonStyle> style = std::nullopt);
```
The `GUIInputState` tells the button the current state of the mouse, active element, etc.

The `ElementID` tells the button its own ID, which can be compared with the currently active ID. If these are equal, then the button is active/being held.

The other data simply specifies the button's appearance.

Each time a GUI element is created, the `GUIContext` will test if the element has just been activated. If so, the currently active ID will be set to this element's ID. This active ID will be retained across frames until a new element is activated or the active element ID is reset.

### Insertion Order ID
The IDs of each element in the GUI rely on insertion order. This allows for very fast ID "calculations" and lookups, as it is simply an index into a vector. However, this does mean that if the UI is modified while an element is active, the active ID may be invalidated. This will occur when the ID representing a particular element changes, which would occur when the number of elements created before the active element changes.

This seems fragile in theory, but is not too bad in practice, as I was not making complex menus. Each time the menu changes, I can just force the active ID to reset to its default value (max uint64_t), which means that any bugs due to incorrect active IDs should not occur.

If I were making anything more than simple one-panel button/slider/checkbox menus, I would definitely use hashes for element IDs, e.g. using element type, position, string etc. But this is obviously going to be slower to calculate and lookup/query elements - so while it is a lot more stable, it was not needed, so the added complexity/compute would not have been worth it.

### Controller Support
Controller navigation is done externally in a class, which increments/decrements the currently active element ID when the up/down controller direction is pressed. A left click is then simulated when the A button is pressed in order to activate the UI element.

Some GUI elements do check for controller activation internally for more specific behaviours, e.g. a `TextEnter` will open the Steam on-screen keyboard if a controller is being used when activated, and a `ColorWheel` checks for controller analogue stick direction when active and moves the color selection position using that input.
