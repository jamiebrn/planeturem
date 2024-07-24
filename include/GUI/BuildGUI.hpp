#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

#include "Object/ObjectBuilder.hpp"
#include "BuildRecipes.hpp"

class BuildGUI
{
    BuildGUI() = delete;

public:
    static void changeSelectedItem(int change);

    static void draw(sf::RenderWindow& window);

private:
    static int selectedItem;

};