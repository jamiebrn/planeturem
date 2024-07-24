#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "Core/TextDraw.hpp"
#include "Object/ObjectBuilder.hpp"
#include "Data/BuildRecipes.hpp"

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