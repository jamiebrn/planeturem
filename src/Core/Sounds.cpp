#include "Core/Sounds.hpp"

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
        sf::SoundBuffer soundBuffer;

        // Load sound data from file stream - set loaded sound to false and stop loading if failed
        if (!soundBuffer.loadFromFile(soundPath))
        {
            // Set loaded sounds to false
            loadedSounds = false;
            // Stop loading
            break;
        }

        // Store sound buffer in map
        soundBufferMap[soundType] = soundBuffer;

        // Create a sound object to interface with the sound buffer
        sf::Sound sound;
        // Set the buffer object of the sound object to the sound buffer in the map
        sound.setBuffer(soundBufferMap[soundType]);

        // Store sound object in map
        soundMap[soundType] = sound;

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
        std::unique_ptr<sf::Music> music = std::make_unique<sf::Music>();

        // Attempt to load the music stream object into the music object
        if (!music->openFromFile(musicPath))
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
    soundBufferMap.clear();

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

    sf::Sound& sound = soundMap.at(type);

    sound.setVolume(volume);

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

    sf::Music* music = musicMap.at(type).get();

    currentlyPlayingMusic = type;

    music->setVolume(volume * musicVolume / 100.0f);
    
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

    sf::Music* music = musicMap.at(type).get();

    return (music->getStatus() == sf::Sound::Stopped);
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
        musicMap[currentlyPlayingMusic.value()]->setVolume(musicVolume);
    }
}