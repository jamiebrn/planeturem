#include "GUI/InventoryGUI.hpp"

sf::Vector2f InventoryGUI::screenPos;

void InventoryGUI::draw(sf::RenderWindow& window)
{
    // Get resolution
    const sf::Vector2u& resolution = ResolutionHandler::getResolution();

    // Draw background
    sf::RectangleShape background({800, 400});

    background.setOrigin({400, 200});
    background.setPosition({resolution.x / 2.0f, resolution.y / 2.0f});
    background.setFillColor({40, 40, 40, 130});

    window.draw(background);

    // Draw items

    sf::Vector2f itemBoxPosition(resolution.x / 2.0f - 400 + 10, resolution.y / 2.0f - 200 + 10);

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
            {3, 3},
            {0.5, 0.5}
        }, ItemDataLoader::getItemData(itemPair.first).textureRect);

        TextDraw::drawText(window, {std::to_string(itemPair.second), itemBoxPosition + sf::Vector2f(70, 70), {255, 255, 255}, 24, {0, 0, 0}, 0, true, true});

        itemBoxPosition.x += 100;
        if (itemBoxPosition.x > resolution.x / 2.0f - 400 + 10 + 7 * 100)
        {
            itemBoxPosition.x = resolution.x / 2.0f - 400 + 10;
            itemBoxPosition.y += 100;
        }
    }
}