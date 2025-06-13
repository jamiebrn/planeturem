#pragma once

#include <Audio/Sound.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <optional>

#include "Core/Tween.hpp"

// Enum containing all sound effects
enum class SoundType
{
    HitObject, HitObject2, HitObject3,
    HitAnimal, HitAnimal2, HitAnimal3,
    CraftBuild1, CraftBuild2,
    Pop0, Pop1, Pop2, Pop3,
    Notify0,
    UIClick0,
    InventoryClick1, InventoryClick2, InventoryClick3,
    InventoryStack1, InventoryStack2,
    Crow
};

// Enum containing all music tracks
enum class MusicType
{
    WorldTheme,
    WorldTheme2,
    BossTheme1
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

    static void update(float dt);

    // Play sound effect
    static void playSound(SoundType type, float volume = 100.0f);

    // Play music track
    static void playMusic(MusicType type, float volume = 100.0f, float fadeTimeForCurrentMusic = 1.0f);

    // Stop music track
    static void stopMusic(float fadeTime = 1.0f);
    // static void stopMusic(MusicType type);

    static bool isMusicFinished();
    static bool isMusicFinished(MusicType type);

    static int getMusicVolume();
    static void setMusicVolume(int volume);
    
    static int getSoundVolume();
    static void setSoundVolume(int volume);

    inline static std::optional<MusicType> getPlayingMusic() {return currentlyPlayingMusic;}

// Private member variables
private:
    // Variable keeping track of whether sounds are loaded into memory
    inline static bool loadedSounds = false;

    inline static int musicVolume = 100;
    inline static std::optional<MusicType> currentlyPlayingMusic = std::nullopt;
    inline static std::optional<MusicType> fadingOutMusic = std::nullopt;
    static float fadingMusicVolume;
    static Tween<float> fadeOutTween;

    inline static int soundVolume = 100;

    // Map storing buffers, which store sound effect data
    // inline static std::unordered_map<SoundType, sf::SoundBuffer> soundBufferMap;

    // Map storing sound objects which interface the sound buffers
    static std::unordered_map<SoundType, std::unique_ptr<pl::Sound>> soundMap;

    // Constant map storing file paths for all sound effects
    static const std::unordered_map<SoundType, std::string> soundPaths;

    // Map storing music objects, which interface with the music streams
    static std::unordered_map<MusicType, std::unique_ptr<pl::Sound>> musicMap;

    // Constant map storing file paths for all music tracks
    static const std::unordered_map<MusicType, std::string> musicPaths;

};