#include "GUI/BuildGUI.hpp"

int BuildGUI::selectedItem = 0;

void BuildGUI::changeSelectedItem(int change)
{
    selectedItem = std::max(selectedItem + change, 0);
}

void BuildGUI::draw(sf::RenderWindow& window)
{
    // Draw background
    sf::RectangleShape background({800, 100});

    background.setOrigin({400, 50});
    background.setPosition({1280 / 2, 650});
    background.setFillColor({40, 40, 40, 130});

    window.draw(background);

    // Draw recipes
    int recipeIndex = 0;
    sf::Vector2f recipeBoxPosition(1280 / 2 - 390, 650 - 40);

    for (auto& recipePair : BuildRecipes)
    {
        // Draw background box
        sf::RectangleShape recipeBox({80, 80});
        recipeBox.setPosition(recipeBoxPosition);
        recipeBox.setFillColor({40, 40, 40, 140});

        window.draw(recipeBox);

        // Draw recipe object
        ObjectType type = recipePair.first;

        std::unique_ptr<WorldObject> recipeObject = createObjectFromType(type, recipeBoxPosition + sf::Vector2f(40, 40));

        recipeObject->draw(window, 0, 255);

        // Draw recipe data if object selected to build
        if (selectedItem == recipeIndex)
        {
            sf::RectangleShape recipeItemsBox({400, 68});

            recipeItemsBox.setOrigin({200, 34});
            recipeItemsBox.setPosition({1280 / 2, 556});
            recipeItemsBox.setFillColor({40, 40, 40, 130});

            window.draw(recipeItemsBox);

            
        }

        recipeIndex++;
        recipeBoxPosition.x += 100;
    }

}