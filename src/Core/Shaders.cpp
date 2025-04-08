#include "Core/Shaders.hpp"

const std::unordered_map<ShaderType, ShaderFilePath> Shaders::shaderFilePaths = {
    {ShaderType::Default, {"Data/Shaders/default.vert", "Data/Shaders/default.frag"}},
    {ShaderType::DefaultNoTexture, {"Data/Shaders/default.vert", "Data/Shaders/default_no_texture.frag"}},
    {ShaderType::TileMap, {"Data/Shaders/tilemap.vert", "Data/Shaders/default.frag"}},
    {ShaderType::Flash, {"Data/Shaders/default.vert", "Data/Shaders/flash.frag"}},
    {ShaderType::Water, {"Data/Shaders/default.vert", "Data/Shaders/water.frag"}},
    // {ShaderType::Lighting, {"", "Data/Shaders/lighting.frag"}},
    {ShaderType::Progress, {"Data/Shaders/default.vert", "Data/Shaders/progress.frag"}},
    {ShaderType::ProgressCircle, {"Data/Shaders/default.vert", "Data/Shaders/progress_circle.frag"}},
    // {ShaderType::Blur, {"", "Data/Shaders/blur.frag"}},
    {ShaderType::ReplaceColour, {"Data/Shaders/default.vert", "Data/Shaders/replace_colour.frag"}}
};
std::unordered_map<ShaderType, std::unique_ptr<pl::Shader>> Shaders::loadedShaders;

bool Shaders::loadShaders()
{
    // Set loaded to true by default
    bool loaded = true;

    for (const std::pair<ShaderType, ShaderFilePath>& shaderPair : shaderFilePaths)
    {
        ShaderType shaderType = shaderPair.first;
        const ShaderFilePath& filePaths = shaderPair.second;

        std::unique_ptr<pl::Shader> shader = std::make_unique<pl::Shader>();

        if (!shader->load(filePaths.vertexPath, filePaths.fragmentPath))
        {
            break;
        }
        
        loadedShaders[shaderType] = std::move(shader);
    }

    // If textures not loaded successfully, return false
    if (!loaded)
        return false;
    
    // Return true by default
    return true;
}