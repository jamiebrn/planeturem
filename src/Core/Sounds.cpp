#include "Core/Sounds.hpp"

std::unordered_map<SoundType, std::unique_ptr<pl::Sound>> Sounds::soundMap;

const std::unordered_map<SoundType, std::string> Sounds::soundPaths = {
    {SoundType::HitObject, "Data/Sounds/hit_object.ogg"},
    {SoundType::HitObject2, "Data/Sounds/hit_object_2.ogg"},
    {SoundType::HitObject3, "Data/Sounds/hit_object_3.ogg"},
    {SoundType::HitAnimal, "Data/Sounds/animal_hit.ogg"},
    {SoundType::HitAnimal2, "Data/Sounds/animal_hit2.ogg"},
    {SoundType::HitAnimal3, "Data/Sounds/animal_hit3.ogg"},
    {SoundType::CraftBuild1, "Data/Sounds/craftbuild1.ogg"},
    {SoundType::CraftBuild2, "Data/Sounds/craftbuild2.ogg"},
    {SoundType::Pop0, "Data/Sounds/pop0.ogg"},
    {SoundType::Pop1, "Data/Sounds/pop1.ogg"},
    {SoundType::Pop2, "Data/Sounds/pop2.ogg"},
    {SoundType::Pop3, "Data/Sounds/pop3.ogg"},
    {SoundType::Notify0, "Data/Sounds/notify0.ogg"},
    {SoundType::UIClick0, "Data/Sounds/uiclick0.ogg"},
    {SoundType::InventoryClick1, "Data/Sounds/inventory-click1.ogg"},
    {SoundType::InventoryClick2, "Data/Sounds/inventory-click2.ogg"},
    {SoundType::InventoryClick3, "Data/Sounds/inventory-click3.ogg"},
    {SoundType::InventoryStack1, "Data/Sounds/inventory-stack1.ogg"},
    {SoundType::InventoryStack2, "Data/Sounds/inventory-stack2.ogg"},
    {SoundType::Crow, "Data/Sounds/crow.ogg"}
};

std::unordered_map<MusicType, std::unique_ptr<pl::Sound>> Sounds::musicMap;

const std::unordered_map<MusicType, std::string> Sounds::musicPaths = {
    {MusicType::WorldTheme, "Data/Sounds/world_theme.ogg"},
    {MusicType::WorldTheme2, "Data/Sounds/world_theme_2.ogg"},
    {MusicType::BossTheme1, "Data/Sounds/boss_theme_1.ogg"}
};

float Sounds::fadingMusicVolume;
Tween<float> Sounds::fadeOutTween;

bool Sounds::loadSounds()
{
    // If sound have already been loaded, return true by default
    if (loadedSounds)
        return true;
    
    // Set loaded sounds to true by default
    loadedSounds = true;

    // Count of loaded sounds (uses float in order to perform float division, rather than casting from int to flat)
    float soundsLoaded = 0;

    // Iterate over every sound effect and its file path, and attempt to load it
    for (std::pair<SoundType, std::string> soundPair : soundPaths)
    {
        // Get sound type and file path from map
        SoundType soundType = soundPair.first;
        std::string soundPath = soundPair.second;

        // Create sound buffer object to store file stream data
        // sf::SoundBuffer soundBuffer;

        std::unique_ptr<pl::Sound> sound = std::make_unique<pl::Sound>();

        // Load sound data from file stream - set loaded sound to false and stop loading if failed
        if (!sound->loadFromFile(soundPath))
        {
            // Set loaded sounds to false
            loadedSounds = false;
            // Stop loading
            break;
        }

        // Store sound buffer in map
        // soundBufferMap[soundType] = soundBuffer;

        // Create a sound object to interface with the sound buffer
        // sf::Sound sound;
        // Set the buffer object of the sound object to the sound buffer in the map
        // sound.setBuffer(soundBufferMap[soundType]);

        // Store sound object in map
        soundMap[soundType] = std::move(sound);

        // Increment sounds loaded count
        soundsLoaded++;
    }

    // Iterate over every music track and its file path, and attempt to load it
    for (std::pair<MusicType, std::string> musicPair : musicPaths)
    {
        // Get music type and file path from map
        MusicType musicType = musicPair.first;
        std::string musicPath = musicPair.second;

        // Create music object (on heap as music object is non-copyable)
        std::unique_ptr<pl::Sound> music = std::make_unique<pl::Sound>();

        // Attempt to load the music stream object into the music object
        if (!music->loadFromFile(musicPath))
        {
            // If failed, set loaded sounds to false
            loadedSounds = false;
            // Stop loading music
            break;
        }

        // Enable music looping and set volume
        // music->setLoop(true);
        // music->setVolume(musicVolume);

        // Move music pointer into map (must be moved as is a unique_ptr, i.e. cannot be copied)
        musicMap[musicType] = std::move(music);

        // Increment sounds loaded
        soundsLoaded++;
    }

    // If loaded sounds is false (unsuccessful load), return false
    if (!loadedSounds)
        return false;
    
    // Return true by default
    return true;
}

// Unload all sounds from memory
void Sounds::unloadSounds()
{
    // Set loaded sounds to false, as they are about to be unloaded
    loadedSounds = false;

    // Delete all sound objects
    soundMap.clear();
    // Delete all sound buffers (must be deleted after sound objects)
    // soundBufferMap.clear();

    // Delete all music objects
    musicMap.clear();
}

void Sounds::update(float dt)
{
    fadeOutTween.update(dt);

    // Test music has finished fading out if required
    if (fadingOutMusic.has_value())
    {
        musicMap.at(fadingOutMusic.value())->setVolume(fadingMusicVolume);

        if (fadingMusicVolume <= 0)
        {
            // Music has finished fading out
            musicMap.at(fadingOutMusic.value())->stop();
            fadingOutMusic = std::nullopt;
        }
    }
}

// Play sound effect
void Sounds::playSound(SoundType type, float volume)
{
    // If sounds have not been loaded, return by default
    if (!loadedSounds)
        return;

    pl::Sound& sound = *soundMap.at(type).get();

    sound.setVolume(volume / 100.0f);

    // Play sound from sound map
    sound.play();
}

// Play music track
void Sounds::playMusic(MusicType type, float volume, float fadeTimeForCurrentMusic)
{
    // If sounds have not been loaded, return by default
    if (!loadedSounds)
        return;
    
    // Stop all music tracks
    if (currentlyPlayingMusic.has_value())
    {
        // If attempt to start same track, do not start again
        if (currentlyPlayingMusic.value() == type)
        {
            return;
        }

        stopMusic(fadeTimeForCurrentMusic);
        // musicMap[currentlyPlayingMusic.value()]->stop();
    }

    pl::Sound* music = musicMap.at(type).get();

    currentlyPlayingMusic = type;

    music->setVolume(volume / 100.0f * musicVolume / 100.0f);
    
    // Play music track from map
    music->play();
}

void Sounds::stopMusic(float fadeTime)
{
    if (!currentlyPlayingMusic.has_value())
    {
        return;
    }

    // Set currently playing music to fade out
    fadingOutMusic = currentlyPlayingMusic;

    fadingMusicVolume = musicMap.at(fadingOutMusic.value())->getVolume();

    // Fade out music
    fadeOutTween.startTween(&fadingMusicVolume, fadingMusicVolume, 0.0f, fadeTime, TweenTransition::Linear, TweenEasing::EaseInOut);

    currentlyPlayingMusic = std::nullopt;
}

// Stop music track
// void Sounds::stopMusic(MusicType type)
// {
//     // If sounds have not been loaded, return by default
//     if (!loadedSounds)
//         return;
    
//     // Stop music track from map
//     musicMap.at(type)->stop();

//     currentlyPlayingMusic = std::nullopt;
// }

bool Sounds::isMusicFinished()
{
    if (!currentlyPlayingMusic.has_value())
    {
        return true;
    }
    return isMusicFinished(currentlyPlayingMusic.value());
}

bool Sounds::isMusicFinished(MusicType type)
{
    if (!loadedSounds)
        return false;

    pl::Sound* music = musicMap.at(type).get();

    return (music->isFinished());
}

int Sounds::getMusicVolume()
{
    return musicVolume;
}

void Sounds::setMusicVolume(int volume)
{
    musicVolume = volume;

    if (currentlyPlayingMusic.has_value())
    {
        musicMap[currentlyPlayingMusic.value()]->setVolume(musicVolume / 100.0f);
    }
}