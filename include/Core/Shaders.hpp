#pragma once


#include <Graphics/Shader.hpp>
#include <unordered_map>
#include <string>
#include <memory>

enum class ShaderType
{
    Default,
    DefaultNoTexture,
    TileMap,
    Flash,
    Water,
    Lighting,
    Progress,
    ProgressCircle,
    MiniMap,
    ReplaceColour
};

struct ShaderFilePath
{
    std::string vertexPath;
    std::string fragmentPath;
};

class Shaders
{
    Shaders() = delete;

public:
    static bool loadShaders();

    static inline pl::Shader* getShader(ShaderType type) {return loadedShaders[type].get();}

private:
    static const std::unordered_map<ShaderType, ShaderFilePath> shaderFilePaths;
    static std::unordered_map<ShaderType, std::unique_ptr<pl::Shader>> loadedShaders;

};