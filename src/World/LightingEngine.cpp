#include "World/LightingEngine.hpp"

void LightingEngine::resize(int width, int height)
{
    this->width = width;
    this->height = height;
    
    // lightingVertexArray.reserve(width * height * 4);
    
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
    // std::queue<LightPropagationNode> lightQueue;
    std::vector<LightPropagationNode> lightQueue;
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

        lightQueue.push_back(LightPropagationNode{i, 0});
    }

    // const float propagationMult = 0.87f;
    const float lightThreshold = 0.01f;

    const int maxDownCheckIndex = width * (height - 1) - 1;

    int processIdx = 0;

    // Process light queue
    while (processIdx < lightQueue.size())
    {
        const LightPropagationNode lightNode = lightQueue[processIdx];
        const float lightIntensity = lighting[lightNode.index];
        // const float nextLightIntensity = lightIntensity * propagationMult;

        processIdx++;

        if (lightIntensity < lightThreshold)
        {
            // lightQueue.pop();
            continue;
        }

        int xIndex = lightNode.index % width;

        // Check left
        if (xIndex > 0)
        {
            propagateLight(LightPropagationNode{lightNode.index - 1, lightNode.steps + 1}, lightIntensity, lightQueue);
        }

        // Check right
        if (xIndex < width - 1)
        {
            propagateLight(LightPropagationNode{lightNode.index + 1, lightNode.steps + 1}, lightIntensity, lightQueue);
        }

        // Check up
        if (lightNode.index >= width)
        {
            propagateLight(LightPropagationNode{lightNode.index - width, lightNode.steps + 1}, lightIntensity, lightQueue);
        }

        // Check down
        if (lightNode.index < maxDownCheckIndex)
        {
            propagateLight(LightPropagationNode{lightNode.index + width, lightNode.steps + 1}, lightIntensity, lightQueue);
        }

        // lightQueue.pop();
    }

    // buildVertexArray(lightingColor);
    generateLightingTexture();
}

void LightingEngine::propagateLight(const LightPropagationNode& lightNode, float previousIntensity, std::vector<LightPropagationNode>& lightQueue)
{
    float stepBaseMult = 0.90f;
    
    #if (!RELEASE_BUILD)
    stepBaseMult = DebugOptions::lightPropMult;
    #endif

    float lightIntensity = previousIntensity * std::pow(stepBaseMult, lightNode.steps);
    float lightIntensityAbsorbed = lightIntensity * (1.0f - obstacles[lightNode.index]);

    if (lighting[lightNode.index] < lightIntensityAbsorbed)
    {
        lighting[lightNode.index] = lightIntensityAbsorbed;
        lightQueue.push_back(LightPropagationNode{lightNode.index});
    }
    // lighting[index] = std::max(lighting[index] + lightIntensityAbsorbed, 1.0f);
    // lightQueue.emplace(index);
}

// void LightingEngine::buildVertexArray(const pl::Color& lightingColor)
// {
//     lightingVertexArray.clear();

//     for (int i = 0; i < lighting.size(); i++)
//     {
//         const float& intensity = lighting[i];
//         if (intensity <= 0)
//         {
//             continue;
//         }

//         int y = static_cast<int>(std::floor(i / width));
//         int x = i % width;

//         pl::Color color(lightingColor.r * intensity, lightingColor.g * intensity, lightingColor.b * intensity);

//         lightingVertexArray.addQuad(pl::Rect<float>(x, y, 1, 1), color, pl::Rect<float>(0, 0, 0, 0));

//         // lightingVertexArray.push_back(sf::Vertex(sf::Vector2f(x, y), colour));
//         // lightingVertexArray.push_back(sf::Vertex(sf::Vector2f(x + 1, y), colour));
//         // lightingVertexArray.push_back(sf::Vertex(sf::Vector2f(x + 1, y + 1), colour));
//         // lightingVertexArray.push_back(sf::Vertex(sf::Vector2f(x, y + 1), colour));
//     }
// }

void LightingEngine::generateLightingTexture()
{
    if (lightingTexture.getID() == 0)
    {
        GLuint textureId;
        glGenTextures(1, &textureId);
        pl::Texture::bindTextureID(textureId, 0);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        lightingTexture.setFromAllocated(textureId, width, height);
        lightingTexture.setLinearFilter(false);
    }

    lightingTexture.overwriteData(width, height, lighting.data(), GL_RED, GL_FLOAT);
}

// void LightingEngine::drawObstacles(pl::RenderTarget& window, int scale)
// {
//     static const sf::Color wallColour = sf::Color(122, 48, 69);
//     static const float ambientLight = 0.3f;

//     sf::VertexArray obstacleVertexArray;

//     for (int i = 0; i < obstacles.size(); i++)
//     {
//         const float& absorption = obstacles[i];
//         if (absorption <= 0)
//         {
//             continue;
//         }

//         const float& lightIntensity = std::max(lighting[i], ambientLight);

//         int y = static_cast<int>(std::floor(i / width));
//         int x = i % width;

//         obstacleVertexArray.append(sf::Vertex(sf::Vector2f(x, y) * (float)scale, wallColour));
//         obstacleVertexArray.append(sf::Vertex(sf::Vector2f(x + 1, y) * (float)scale, wallColour));
//         obstacleVertexArray.append(sf::Vertex(sf::Vector2f(x + 1, y + 1) * (float)scale, wallColour));
//         obstacleVertexArray.append(sf::Vertex(sf::Vector2f(x, y + 1) * (float)scale, wallColour));
//     }
    
//     if (obstacleVertexArray.getVertexCount() <= 0)
//     {
//         return;
//     }

//     window.draw(&(obstacleVertexArray[0]), obstacleVertexArray.getVertexCount(), sf::Quads);
// }

void LightingEngine::drawLighting(pl::RenderTarget& window, const pl::Color& lightingColor)
{
    if (lightingTexture.getID() <= 0)
    {
        return;
    }

    pl::VertexArray lightingRect;
    lightingRect.addQuad(pl::Rect<float>(0, 0, window.getWidth(), window.getHeight()), lightingColor, pl::Rect<float>(0, 0, width, height));

    window.draw(lightingRect, *Shaders::getShader(ShaderType::Lighting), &lightingTexture, pl::BlendMode::Add);

    // window.draw(&(lightingVertexArray[0]), lightingVertexArray.size(), sf::Quads, sf::BlendAdd);
}