#pragma once

#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>

#include <Graphics/VertexArray.hpp>
#include <Graphics/Color.hpp>
#include <Graphics/RenderTarget.hpp>
#include <Graphics/Texture.hpp>
#include <Vector.hpp>
#include <Rect.hpp>

#include "Core/Shaders.hpp"

#include "GameConstants.hpp"
#include "DebugOptions.hpp"

class LightingEngine
{
public:
    LightingEngine() = default;
    ~LightingEngine() = default;

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

    // void drawObstacles(pl::RenderTarget& window, int scale);

    void drawLighting(pl::RenderTarget& window, const pl::Color& lightingColor);

private:
    // void buildVertexArray(const pl::Color& lightingColor);
    void generateLightingTexture();

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

    // pl::VertexArray lightingVertexArray;
    pl::Texture lightingTexture;

    int width;
    int height;

};