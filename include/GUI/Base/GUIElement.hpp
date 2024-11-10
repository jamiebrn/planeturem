#pragma once

#include <SFML/Graphics.hpp>

#include "GUIInputState.hpp"

class GUIElement
{
public:
    inline GUIElement(ElementID id) {this->id = id;};
    
    virtual void draw(sf::RenderTarget& window) = 0;

    virtual sf::IntRect getBoundingBox() const = 0;

    bool isHovered() const {return hovered;} 

    inline ElementID getElementID() const {return id;}

protected:
    ElementID id;

    bool hovered = false;

};