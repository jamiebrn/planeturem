#pragma once

#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>

// #include <SFML/Graphics.hpp>

#include <Graphics/Vertex.hpp>
#include <Graphics/VertexArray.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Shaders.hpp"

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

    void calculateLighting(const pl::Color& lightingColor);

    // void drawObstacles(pl::RenderTarget& window, int scale);

    void drawLighting(pl::RenderTarget& window);

private:
    void buildVertexArray(const pl::Color& lightingColor);

private:
    struct LightPropagationNode
    {
        int index;
        int steps;
    };

    void propagateLight(const LightPropagationNode& lightNode, float previousIntensity, std::queue<LightPropagationNode>& lightQueue);

private:
    std::vector<float> lighting;
    std::vector<float> movingLightSources;
    std::vector<float> lightSources;
    std::vector<float> obstacles;

    // std::vector<sf::Vertex> lightingVertexArray;

    pl::VertexArray lightingVertexArray;

    int width;
    int height;

};