#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <memory>

enum ShaderType
{
    Flash
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

    static inline sf::Shader* getShader(ShaderType type) {return loadedShaders[type].get();}

private:
    static const std::unordered_map<ShaderType, ShaderFilePath> shaderFilePaths;
    static std::unordered_map<ShaderType, std::unique_ptr<sf::Shader>> loadedShaders;

};