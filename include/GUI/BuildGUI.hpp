#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "TextDraw.hpp"
#include "Object/ObjectBuilder.hpp"
#include "BuildRecipes.hpp"

class BuildGUI
{
    BuildGUI() = delete;

public:
    static void changeSelectedObject(int change);

    static ObjectType getSelectedObject();

    static void draw(sf::RenderWindow& window);

private:
    static int selectedItem;

};