#pragma once
#include "raylib.h"
#include "Config.h"

// Efekt dźwiękowy, który może nie istnieć (wtedy po prostu nic nie gra).
struct Sfx
{
    Sound sound{};
    bool  loaded = false;

    void Load(const char* path);
    void Play() const;
    void Unload();
};

// Wszystkie obrazki i dźwięki w jednym miejscu.
// Brakujący plik nie wywala gry - zostanie narysowany zastępnik.
struct Assets
{
    Texture2D menuBackground{};
    Texture2D storyBackgrounds[STORY_PAGES]{};
    Texture2D levelBackgrounds[LEVEL_COUNT]{};
    Texture2D endingBackground{};
    Texture2D knight{};
    Texture2D goblin{};

    Music music{};
    bool  musicLoaded = false;
    Sfx   click, win, lose;

    void Load();
    void Unload();
    void ApplyMusicVolume(float volume);   // 0.0 - 1.0
};
