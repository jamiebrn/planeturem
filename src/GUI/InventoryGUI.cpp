#include "GUI/InventoryGUI.hpp"

void InventoryGUI::draw(sf::RenderWindow& window)
{
    // Draw background
    sf::RectangleShape background({800, 400});

    background.setOrigin({400, 200});
    background.setPosition({1280 / 2, 720 / 2});
    background.setFillColor({40, 40, 40, 130});

    window.draw(background);

    // Draw items

    sf::Vector2f itemBoxPosition(1280 / 2 - 400 + 10, 720 / 2 - 200 + 10);

    for (auto& itemPair : Inventory::getData())
    {
        sf::RectangleShape itemBackground({80, 80});

        // itemBackground.setOrigin({40, 40});
        itemBackground.setPosition(itemBoxPosition);
        itemBackground.setFillColor({40, 40, 40, 140});

        window.draw(itemBackground);

        TextureManager::drawSubTexture(window, {
            TextureType::Items,
            itemBoxPosition + sf::Vector2f(40, 40),
            0,
            3,
            {0.5, 0.5}
        }, ItemDataLoader::getItemData(itemPair.first).textureRect);

        TextDraw::drawText(window, {std::to_string(itemPair.second), itemBoxPosition + sf::Vector2f(70, 70), {255, 255, 255}, 24, {0, 0, 0}, 0, true, true});

        itemBoxPosition.x += 100;
        if (itemBoxPosition.x > 1280 / 2 - 400 + 10 + 7 * 100)
        {
            itemBoxPosition.x = 1280 / 2 - 400 + 10;
            itemBoxPosition.y += 100;
        }
    }
}