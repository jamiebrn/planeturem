#pragma once

#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>

#include <SFML/Graphics.hpp>

class LightingEngine
{
public:
    LightingEngine() = default;

    void resize(int width, int height);

    int getWidth();
    int getHeight();

    void resetLighting();

    void resetLightSources();
    void resetObstacles();

    void addLightSource(int x, int y, float intensity);

    void addMovingLightSource(int x, int y, float intensity);

    void addObstacle(int x, int y, float absorption = 1.0f);

    void calculateLighting();

    void drawObstacles(sf::RenderTarget& window);

    void drawLighting(sf::RenderTarget& window, const sf::Color& lightingColour);

private:
    void propagateLight(int index, float lightIntensity, std::queue<int>& lightQueue);

private:
    std::vector<float> lighting;
    std::vector<float> movingLightSources;
    std::vector<float> lightSources;
    std::vector<float> obstacles;

    std::vector<sf::Vertex> lightingVertexArray;

    int width;
    int height;

};