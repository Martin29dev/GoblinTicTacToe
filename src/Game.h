#pragma once
#include <functional>
#include "raylib.h"
#include "Assets.h"
#include "Level.h"
#include "Settings.h"

enum class Scene { Menu, Settings, Story, Level, Ending };
enum class FadePhase { None, Out, In };   // None = brak przejścia, Out = ściemnianie, In = rozjaśnianie

// Cała gra: okno, ekrany (scenes), przejścia i muzyka.
class Game
{
public:
    Game();
    ~Game();
    void Run();

private:
    // pętla
    void UpdateLetterbox();
    void Update();
    void DrawScene();

    // ekrany
    void UpdateStory();
    void UpdateLevel();
    void DrawMenu();
    void DrawSettings();
    void DrawStory() const;
    void DrawEnding();

    // przejścia (fade out -> zmiana -> fade in)
    void StartTransition(std::function<void()> onBlack);
    void UpdateFade();
    void DrawFadeOverlay() const;
    void GoToScene(Scene next);
    void StartLevel(int index);

    bool Button(Rectangle rect, const char* text);   // przycisk + dźwięk kliknięcia
    void ApplyMusicVolume();

    Scene    scene = Scene::Menu;
    Settings settings;
    Assets   assets;
    LevelScreen level;
    int   currentLevel = 0;
    int   storyPage = 0;
    float storyTimer = 0.0f;
    bool  shouldExit = false;

    RenderTexture2D canvas{};     // gra rysuje do stałych 960x540, potem skalujemy do okna
    Rectangle letterbox{};

    FadePhase fadePhase = FadePhase::In;
    float fadeAlpha = 1.0f;
    std::function<void()> onBlack;
};
