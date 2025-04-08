#pragma once

// Include libraries
// #include <SFML/Audio.hpp>
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
    CraftBuild1, CraftBuild2,
    Pop0, Pop1, Pop2, Pop3,
    Notify0,
    UIClick0,
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

    inline static std::optional<MusicType> getPlayingMusic() {return currentlyPlayingMusic;}

// Private member variables
private:
    // Variable keeping track of whether sounds are loaded into memory
    inline static bool loadedSounds = false;

    inline static int musicVolume = 100.0f;
    inline static std::optional<MusicType> currentlyPlayingMusic = std::nullopt;
    inline static std::optional<MusicType> fadingOutMusic = std::nullopt;
    static float fadingMusicVolume;
    static Tween<float> fadeOutTween;

    // Map storing buffers, which store sound effect data
    // inline static std::unordered_map<SoundType, sf::SoundBuffer> soundBufferMap;

    // Map storing sound objects which interface the sound buffers
    inline static std::unordered_map<SoundType, std::unique_ptr<pl::Sound>> soundMap;

    // Constant map storing file paths for all sound effects
    inline static const std::unordered_map<SoundType, std::string> soundPaths = {
        {SoundType::HitObject, "Data/Sounds/hit_object.ogg"},
        {SoundType::HitObject2, "Data/Sounds/hit_object_2.ogg"},
        {SoundType::HitObject3, "Data/Sounds/hit_object_3.ogg"},
        {SoundType::CraftBuild1, "Data/Sounds/craftbuild1.ogg"},
        {SoundType::CraftBuild2, "Data/Sounds/craftbuild2.ogg"},
        {SoundType::Pop0, "Data/Sounds/pop0.ogg"},
        {SoundType::Pop1, "Data/Sounds/pop1.ogg"},
        {SoundType::Pop2, "Data/Sounds/pop2.ogg"},
        {SoundType::Pop3, "Data/Sounds/pop3.ogg"},
        {SoundType::Notify0, "Data/Sounds/notify0.ogg"},
        {SoundType::UIClick0, "Data/Sounds/uiclick0.ogg"},
        {SoundType::Crow, "Data/Sounds/crow.ogg"}
    };

    // Map storing music objects, which interface with the music streams
    inline static std::unordered_map<MusicType, std::unique_ptr<pl::Sound>> musicMap;

    // Constant map storing file paths for all music tracks
    inline static const std::unordered_map<MusicType, std::string> musicPaths = {
        {MusicType::WorldTheme, "Data/Sounds/world_theme.ogg"},
        {MusicType::WorldTheme2, "Data/Sounds/world_theme_2.ogg"},
        {MusicType::BossTheme1, "Data/Sounds/boss_theme_1.ogg"}
    };

};