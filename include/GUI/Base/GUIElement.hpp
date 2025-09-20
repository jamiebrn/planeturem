#pragma once

#include <Graphics/RenderTarget.hpp>
#include <Rect.hpp>

#include "GUIInputState.hpp"

class GUIElement
{
public:
    inline GUIElement(ElementID id, int textSize) : id(id), textSize(textSize) {}
    virtual ~GUIElement() = default;
    
    virtual void draw(pl::RenderTarget& window) = 0;

    virtual pl::Rect<int> getBoundingBox() const = 0;

    inline bool isHovered() const {return hovered;} 

    inline ElementID getElementID() const {return id;}

protected:
    ElementID id;

    int textSize = 0;

    bool hovered = false;

};