#include "World/LightingEngine.hpp"

void LightingEngine::resize(int width, int height)
{
    this->width = width;
    this->height = height;
    
    lightingVertexArray.reserve(width * height * 4);
    
    resetLighting();
    resetLightSources();
    resetObstacles();
}

int LightingEngine::getWidth()
{
    return width;
}

int LightingEngine::getHeight()
{
    return height;
}

void LightingEngine::resetLighting()
{
    lighting = std::vector<float>(width * height, 0.0f);
    movingLightSources = std::vector<float>(width * height, 0.0f);
}

void LightingEngine::resetLightSources()
{
    lightSources = std::vector<float>(width * height, 0.0f);
}

void LightingEngine::resetObstacles()
{
    obstacles = std::vector<float>(width * height, 0.0f);
}

void LightingEngine::addLightSource(int x, int y, float intensity)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        return;
    }

    lightSources[y * width + x] = intensity;
}

void LightingEngine::addMovingLightSource(int x, int y, float intensity)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        return;
    }

    movingLightSources[y * width + x] = intensity;
}

void LightingEngine::addObstacle(int x, int y, float absorption)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
    {
        return;
    }

    obstacles[y * width + x] = absorption;
}

void LightingEngine::calculateLighting()
{
    // Initialise light sources and put indexes into queue
    std::queue<int> lightQueue;
    for (int i = 0; i < lightSources.size(); i++)
    {
        const float& intensity = std::max(lightSources[i], movingLightSources[i]);
        if (intensity <= 0)
        {
            continue;
        }

        const float& lightAbsorption = obstacles[i];
        if (lightAbsorption >= 1)
        {
            continue;
        }

        // Is light source
        lighting[i] = intensity * (1.0f - lightAbsorption);

        lightQueue.emplace(i);
    }

    const float propagationMult = 0.87f;
    const float lightThreshold = 0.01f;

    const int maxDownCheckIndex = width * (height - 1) - 1;

    // Process light queue
    while (!lightQueue.empty())
    {
        const int lightIndex = lightQueue.front();
        const float lightIntensity = lighting[lightIndex];
        const float nextLightIntensity = lightIntensity * propagationMult;

        if (nextLightIntensity < lightThreshold)
        {
            lightQueue.pop();
            continue;
        }

        int xIndex = lightIndex % width;

        // Check left
        if (xIndex > 0)
        {
            propagateLight(lightIndex - 1, nextLightIntensity, lightQueue);
        }

        // Check right
        if (xIndex < width - 1)
        {
            propagateLight(lightIndex + 1, nextLightIntensity, lightQueue);
        }

        // Check up
        if (lightIndex > width)
        {
            propagateLight(lightIndex - width, nextLightIntensity, lightQueue);
        }

        // Check down
        if (lightIndex < maxDownCheckIndex)
        {
            propagateLight(lightIndex + width, nextLightIntensity, lightQueue);
        }

        lightQueue.pop();
    }
}

void LightingEngine::propagateLight(int index, float lightIntensity, std::queue<int>& lightQueue)
{
    float lightIntensityAbsorbed = lightIntensity * (1.0f - obstacles[index]);
    if (lighting[index] < lightIntensityAbsorbed)
    {
        lighting[index] = lightIntensityAbsorbed;
        lightQueue.emplace(index);
    }
    // lighting[index] = std::max(lighting[index] + lightIntensityAbsorbed, 1.0f);
    // lightQueue.emplace(index);
}

void LightingEngine::drawObstacles(sf::RenderTarget& window, int scale)
{
    static const sf::Color wallColour = sf::Color(122, 48, 69);
    static const float ambientLight = 0.3f;

    sf::VertexArray obstacleVertexArray;

    for (int i = 0; i < obstacles.size(); i++)
    {
        const float& absorption = obstacles[i];
        if (absorption <= 0)
        {
            continue;
        }

        const float& lightIntensity = std::max(lighting[i], ambientLight);

        int y = static_cast<int>(std::floor(i / width));
        int x = i % width;

        obstacleVertexArray.append(sf::Vertex(sf::Vector2f(x, y) * (float)scale, wallColour));
        obstacleVertexArray.append(sf::Vertex(sf::Vector2f(x + 1, y) * (float)scale, wallColour));
        obstacleVertexArray.append(sf::Vertex(sf::Vector2f(x + 1, y + 1) * (float)scale, wallColour));
        obstacleVertexArray.append(sf::Vertex(sf::Vector2f(x, y + 1) * (float)scale, wallColour));
    }
    
    if (obstacleVertexArray.getVertexCount() <= 0)
    {
        return;
    }

    window.draw(&(obstacleVertexArray[0]), obstacleVertexArray.getVertexCount(), sf::Quads);
}

void LightingEngine::drawLighting(sf::RenderTarget& window, const sf::Color& lightingColour)
{
    lightingVertexArray.clear();

    for (int i = 0; i < lighting.size(); i++)
    {
        const float& intensity = lighting[i];
        if (intensity <= 0)
        {
            continue;
        }

        int y = static_cast<int>(std::floor(i / width));
        int x = i % width;

        sf::Color colour(lightingColour.r * intensity, lightingColour.g * intensity, lightingColour.b * intensity);

        lightingVertexArray.push_back(sf::Vertex(sf::Vector2f(x, y), colour));
        lightingVertexArray.push_back(sf::Vertex(sf::Vector2f(x + 1, y), colour));
        lightingVertexArray.push_back(sf::Vertex(sf::Vector2f(x + 1, y + 1), colour));
        lightingVertexArray.push_back(sf::Vertex(sf::Vector2f(x, y + 1), colour));
    }

    if (lightingVertexArray.size() <= 0)
    {
        return;
    }

    window.draw(&(lightingVertexArray[0]), lightingVertexArray.size(), sf::Quads, sf::BlendAdd);
}