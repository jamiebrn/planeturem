#pragma once

// Include libraries
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>

// Enum containing all sound effects
enum class SoundType
{
    
};

// Enum containing all music tracks
enum class MusicType
{
    Main
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
    static void playSound(SoundType type);

    // Play music track
    static void playMusic(MusicType type);

    // Stop music track
    static void stopMusic(MusicType type);

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
    };

    // Map storing music objects, which interface with the music streams
    inline static std::unordered_map<MusicType, std::unique_ptr<sf::Music>> musicMap;

    // Constant storing volume of music
    static constexpr float MUSIC_VOLUME = 10.0f;

    // Constant map storing file paths for all music tracks
    inline static const std::unordered_map<MusicType, std::string> musicPaths = {
        {MusicType::Main, "Data/first tune.ogg"}
    };

};