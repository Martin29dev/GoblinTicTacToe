#include "Assets.h"
#include <string>

namespace
{
    // Foldery z zasobami (względem folderu projektu).
    const std::string PNG_DIR    = "assets/Png/";
    const std::string SOUNDS_DIR = "assets/Sounds/";

    std::string Path(const std::string& file) { return SOUNDS_DIR + file; }   // dźwięki i muzyka

    Texture2D LoadTex(const std::string& file)
    {
        return LoadTexture((PNG_DIR + file).c_str());   // przy braku pliku raylib zwróci id == 0
    }

    void UnloadTex(Texture2D& tex)
    {
        if (tex.id != 0) UnloadTexture(tex);
        tex = Texture2D{};
    }
}

// ---------------------------------------------------------------- Sfx

void Sfx::Load(const char* path)
{
    if (!FileExists(path))
    {
        TraceLog(LOG_WARNING, "Brak dzwieku: %s", path);
        return;
    }
    sound = LoadSound(path);
    loaded = true;
}

void Sfx::Play() const
{
    if (loaded) PlaySound(sound);
}

void Sfx::Unload()
{
    if (loaded) UnloadSound(sound);
    loaded = false;
}

// ---------------------------------------------------------------- Assets

void Assets::Load()
{
    // --- obrazki (nazwy plików w folderze assets) ---
    menuBackground = LoadTex("MenuBackground.png");
    endingBackground = LoadTex("EndingBackground.png");
    knight = LoadTex("Knight.png");
    goblin = LoadTex("Goblin.png");

    storyBackgrounds[0] = LoadTex("FirstStoryBackgroundWithGoblin.png");
    storyBackgrounds[1] = LoadTex("SecondStoryBackgroundWithGoblin.png");
    storyBackgrounds[2] = LoadTex("ThirdStoryBackgroundWithGoblin.png");
    storyBackgrounds[3] = LoadTex("FourthStoryBackgroundWithGoblin.png");

    levelBackgrounds[0] = LoadTex("Level1Background.png");
    levelBackgrounds[1] = LoadTex("Level2Background.png");
    levelBackgrounds[2] = LoadTex("Level3Background.png");

    // --- dźwięki ---
    click.Load(Path("click.wav").c_str());
    win.Load(Path("win.wav").c_str());
    lose.Load(Path("lose.wav").c_str());
    enemyMove.Load(Path("enemy.wav").c_str());
    swish.Load(Path("swish.wav").c_str());

    // --- muzyka: bierzemy pierwszy istniejący plik ---
    const char* musicFiles[] = { "music.mp3", "music.ogg", "music.wav" };
    for (const char* file : musicFiles)
    {
        if (FileExists(Path(file).c_str()))
        {
            music = LoadMusicStream(Path(file).c_str());
            music.looping = true;
            musicLoaded = true;
            break;
        }
    }
    if (!musicLoaded) TraceLog(LOG_WARNING, "Brak muzyki (assets/Sounds/music.mp3)");
}

void Assets::Unload()
{
    UnloadTex(menuBackground);
    UnloadTex(endingBackground);
    UnloadTex(knight);
    UnloadTex(goblin);
    for (auto& tex : storyBackgrounds) UnloadTex(tex);
    for (auto& tex : levelBackgrounds) UnloadTex(tex);

    click.Unload();
    win.Unload();
    lose.Unload();
    enemyMove.Unload();
    swish.Unload();

    if (musicLoaded) UnloadMusicStream(music);
    musicLoaded = false;
}

void Assets::ApplyMusicVolume(float volume)
{
    if (musicLoaded) SetMusicVolume(music, volume);
}
