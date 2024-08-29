#pragma once

// Include libraries
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>

// Enum containing all sound effects
enum class SoundType
{
    HitObject, HitObject2, HitObject3,
    CraftBuild1, CraftBuild2
};

// Enum containing all music tracks
enum class MusicType
{
    WorldTheme,
    WorldTheme2
};

// Declare sound manager class
class Sounds
{

// Delete constructor, as is static class
private:
    Sounds() = delete;

// Public class functions
public:
    // Load all sounds into memory (sound effects and music)
    static bool loadSounds();

    // Unload all sounds from memory
    static void unloadSounds();

    // Play sound effect
    static void playSound(SoundType type, float volume = 100.0f);

    // Play music track
    static void playMusic(MusicType type, float volume = 100.0f);

    // Stop music track
    static void stopMusic(MusicType type);

    static bool isMusicFinished(MusicType type);

// Private member variables
private:
    // Variable keeping track of whether sounds are loaded into memory
    inline static bool loadedSounds = false;

    // Map storing buffers, which store sound effect data
    inline static std::unordered_map<SoundType, sf::SoundBuffer> soundBufferMap;

    // Map storing sound objects which interface the sound buffers
    inline static std::unordered_map<SoundType, sf::Sound> soundMap;

    // Constant map storing file paths for all sound effects
    inline static const std::unordered_map<SoundType, std::string> soundPaths = {
        {SoundType::HitObject, "Data/Sounds/hit_object.ogg"},
        {SoundType::HitObject2, "Data/Sounds/hit_object_2.ogg"},
        {SoundType::HitObject3, "Data/Sounds/hit_object_3.ogg"},
        {SoundType::CraftBuild1, "Data/Sounds/craftbuild1.ogg"},
        {SoundType::CraftBuild2, "Data/Sounds/craftbuild2.ogg"}
    };

    // Map storing music objects, which interface with the music streams
    inline static std::unordered_map<MusicType, std::unique_ptr<sf::Music>> musicMap;

    // Constant map storing file paths for all music tracks
    inline static const std::unordered_map<MusicType, std::string> musicPaths = {
        {MusicType::WorldTheme, "Data/Sounds/world_theme.ogg"},
        {MusicType::WorldTheme2, "Data/Sounds/world_theme_2.ogg"}
    };

};