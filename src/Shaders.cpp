#include "Shaders.hpp"

const std::unordered_map<ShaderType, ShaderFilePath> Shaders::shaderFilePaths = {
    {ShaderType::Flash, {"", "flash.frag"}},
    {ShaderType::Tree, {"", "tree.frag"}}
};
std::unordered_map<ShaderType, std::unique_ptr<sf::Shader>> Shaders::loadedShaders;

bool Shaders::loadShaders()
{
    // Set loaded to true by default
    bool loaded = true;

    for (std::pair<ShaderType, ShaderFilePath> shaderPair : shaderFilePaths)
    {
        ShaderType shaderType = shaderPair.first;
        ShaderFilePath filePaths = shaderPair.second;

        std::unique_ptr<sf::Shader> shader = std::make_unique<sf::Shader>();

        if (filePaths.vertexPath.empty())
        {
            loaded = shader->loadFromFile(filePaths.fragmentPath, sf::Shader::Fragment);
        }
        else if (filePaths.fragmentPath.empty())
        {
            loaded = shader->loadFromFile(filePaths.vertexPath, sf::Shader::Vertex);
        }
        else
        {
            loaded = shader->loadFromFile(filePaths.vertexPath, filePaths.fragmentPath);
        }
        
        if (!loaded)
            break;
        
        loadedShaders[shaderType] = std::move(shader);
    }

    // If textures not loaded successfully, return false
    if (!loaded)
        return false;
    
    // Return true by default
    return true;
}