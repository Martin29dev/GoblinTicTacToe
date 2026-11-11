#include "Game.h"
#include <algorithm>
#include "Config.h"
#include "UI.h"

namespace
{
    const float FADE_SPEED = 2.5f;           // im większe, tym szybsze przejścia
    const float STORY_AUTO_SECONDS = 5.0f;   // po tylu sekundach historia sama idzie dalej

    const char* STORY_TEXT[STORY_PAGES] = {
        "Dawno temu w spokojnym krolestwie zapadla noc...",
        "Podly goblin porwal ksiezniczke do swojej twierdzy!",
        "Odwazny rycerz wyruszyl jej na ratunek.",
        "Goblin zgodzil sie na pojedynek w kolko i krzyzyk!"
    };
}

// ---------------------------------------------------------------- start / koniec

Game::Game()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Goblin TicTacToe Game");
    InitAudioDevice();
    SetExitKey(KEY_NULL);          // Esc nie zamyka gry, tylko wraca do menu
    SetTargetFPS(60);

    canvas = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_BILINEAR);

    assets.Load();
    ApplyMusicVolume();
    if (assets.musicLoaded) PlayMusicStream(assets.music);
}

Game::~Game()
{
    assets.Unload();
    UnloadRenderTexture(canvas);
    CloseAudioDevice();
    CloseWindow();
}

void Game::ApplyMusicVolume()
{
    assets.ApplyMusicVolume(settings.musicVolume / 10.0f);
}

// ---------------------------------------------------------------- pętla główna

void Game::Run()
{
    while (!WindowShouldClose() && !shouldExit)
    {
        UpdateLetterbox();
        Update();

        // 1. Rysujemy całą grę na "płótno" 960x540.
        BeginTextureMode(canvas);
            ClearBackground(BLACK);
            DrawScene();
            DrawFadeOverlay();
        EndTextureMode();

        // 2. Płótno skalujemy do okna (z czarnymi pasami, jeśli proporcje się różnią).
        BeginDrawing();
            ClearBackground(BLACK);
            Rectangle source = { 0.0f, 0.0f, (float)SCREEN_WIDTH, -(float)SCREEN_HEIGHT };  // minus = odwrócenie w pionie
            DrawTexturePro(canvas.texture, source, letterbox, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
        EndDrawing();
    }
}

// Liczy, gdzie płótno leży w oknie, i przelicza pozycję myszy na współrzędne gry.
void Game::UpdateLetterbox()
{
    float scale = std::min(GetScreenWidth()  / (float)SCREEN_WIDTH,
                           GetScreenHeight() / (float)SCREEN_HEIGHT);

    letterbox.width  = SCREEN_WIDTH  * scale;
    letterbox.height = SCREEN_HEIGHT * scale;
    letterbox.x = (GetScreenWidth()  - letterbox.width)  / 2;
    letterbox.y = (GetScreenHeight() - letterbox.height) / 2;

    SetMouseOffset((int)-letterbox.x, (int)-letterbox.y);
    SetMouseScale(1.0f / scale, 1.0f / scale);
}

void Game::Update()
{
    if (assets.musicLoaded) UpdateMusicStream(assets.music);
    if (IsKeyPressed(KEY_F)) ToggleBorderlessWindowed();

    UpdateFade();

    // Podczas przejścia nic nie da się kliknąć.
    const bool canInteract = (fadePhase == FadePhase::None);
    UI::SetEnabled(canInteract);
    if (!canInteract) return;

    if (IsKeyPressed(KEY_ESCAPE) && scene != Scene::Menu)
    {
        GoToScene(Scene::Menu);
        return;
    }

    if (scene == Scene::Story) UpdateStory();
    if (scene == Scene::Level) UpdateLevel();
}

void Game::DrawScene()
{
    switch (scene)
    {
    case Scene::Menu:     DrawMenu();     break;
    case Scene::Settings: DrawSettings(); break;
    case Scene::Story:    DrawStory();    break;
    case Scene::Level:    level.Draw(assets, settings); break;
    case Scene::Ending:   DrawEnding();   break;
    }
}

// ---------------------------------------------------------------- przejścia

// Ściemnia ekran, wykonuje akcję (gdy jest czarno), rozjaśnia ekran.
void Game::StartTransition(std::function<void()> action)
{
    if (fadePhase != FadePhase::None) return;
    onBlack = action;
    fadePhase = FadePhase::Out;
}

void Game::UpdateFade()
{
    const float dt = GetFrameTime();

    if (fadePhase == FadePhase::Out)
    {
        fadeAlpha += FADE_SPEED * dt;
        if (fadeAlpha >= 1.0f)
        {
            fadeAlpha = 1.0f;
            if (onBlack) onBlack();      // ekran jest czarny - można bezpiecznie zmienić scenę
            onBlack = nullptr;
            fadePhase = FadePhase::In;
        }
    }
    else if (fadePhase == FadePhase::In)
    {
        fadeAlpha -= FADE_SPEED * dt;
        if (fadeAlpha <= 0.0f)
        {
            fadeAlpha = 0.0f;
            fadePhase = FadePhase::None;
        }
    }
}

void Game::DrawFadeOverlay() const
{
    if (fadeAlpha > 0.0f)
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, fadeAlpha));
}

void Game::GoToScene(Scene next)
{
    StartTransition([this, next]()
    {
        scene = next;
        if (next == Scene::Story) { storyPage = 0; storyTimer = 0.0f; }
    });
}

void Game::StartLevel(int index)
{
    StartTransition([this, index]()
    {
        currentLevel = index;
        level.Start(index);
        scene = Scene::Level;
    });
}

bool Game::Button(Rectangle rect, const char* text)
{
    bool clicked = UI::Button(rect, text);
    if (clicked) assets.click.Play();
    return clicked;
}

// ---------------------------------------------------------------- menu

void Game::DrawMenu()
{
    UI::DrawBackground(assets.menuBackground, Color{ 25, 20, 60, 255 }, Color{ 80, 40, 35, 255 });

    UI::DrawTextCentered("GOBLIN TIC TAC TOE", SCREEN_WIDTH / 2, 60, 60, GOLD);
    UI::DrawTextCentered("Uratuj ksiezniczke!", SCREEN_WIDTH / 2, 135, 26, RAYWHITE);

    const float x = (SCREEN_WIDTH - 260) / 2.0f;
    if (Button({ x, 220, 260, 56 }, "Start"))      GoToScene(Scene::Story);
    if (Button({ x, 295, 260, 56 }, "Ustawienia")) GoToScene(Scene::Settings);
    if (Button({ x, 370, 260, 56 }, "Wyjscie"))    shouldExit = true;

    UI::DrawTextCentered("[F] pelny ekran", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 35, 18, LIGHTGRAY);
}

// ---------------------------------------------------------------- ustawienia

void Game::DrawSettings()
{
    UI::DrawBackground(assets.menuBackground, Color{ 25, 20, 60, 255 }, Color{ 80, 40, 35, 255 });
    DrawRectangleRounded({ 180, 110, 600, 300 }, 0.08f, 8, Fade(BLACK, 0.55f));

    UI::DrawTextCentered("USTAWIENIA", SCREEN_WIDTH / 2, 40, 48, GOLD);

    // Muzyka: [-] 7/10 [+]
    DrawText("Muzyka", 220, 150, 28, RAYWHITE);
    if (Button({ 520, 140, 50, 50 }, "-") && settings.musicVolume > 0)  { settings.musicVolume--; ApplyMusicVolume(); }
    UI::DrawTextCentered(TextFormat("%d/10", settings.musicVolume), 620, 150, 28, RAYWHITE);
    if (Button({ 670, 140, 50, 50 }, "+") && settings.musicVolume < 10) { settings.musicVolume++; ApplyMusicVolume(); }

    // Statystyki AI
    DrawText("Statystyki AI", 220, 230, 28, RAYWHITE);
    if (Button({ 520, 220, 200, 50 }, settings.showStats ? "TAK" : "NIE"))
        settings.showStats = !settings.showStats;

    // Pełny ekran
    DrawText("Pelny ekran", 220, 310, 28, RAYWHITE);
    if (Button({ 520, 300, 200, 50 }, "Przelacz"))
        ToggleBorderlessWindowed();

    if (Button({ (SCREEN_WIDTH - 260) / 2.0f, 440, 260, 56 }, "Wroc"))
        GoToScene(Scene::Menu);
}

// ---------------------------------------------------------------- historia

void Game::UpdateStory()
{
    storyTimer += GetFrameTime();

    const bool next = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)
                   || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
                   || storyTimer >= STORY_AUTO_SECONDS;
    if (!next) return;

    if (storyPage < STORY_PAGES - 1)
        StartTransition([this]() { storyPage++; storyTimer = 0.0f; });
    else
        StartLevel(0);
}

void Game::DrawStory() const
{
    UI::DrawBackground(assets.storyBackgrounds[storyPage], Color{ 30, 30, 50, 255 }, Color{ 10, 10, 20, 255 });

    DrawRectangle(0, 400, SCREEN_WIDTH, 140, Fade(BLACK, 0.65f));
    UI::DrawTextCentered(STORY_TEXT[storyPage], SCREEN_WIDTH / 2, 430, 30, RAYWHITE);
    UI::DrawTextCentered("[Enter] dalej   [Esc] menu", SCREEN_WIDTH / 2, 495, 18, LIGHTGRAY);
}

// ---------------------------------------------------------------- poziomy i zakończenie

void Game::UpdateLevel()
{
    switch (level.Update(assets))
    {
    case LevelAction::NextLevel:
        if (currentLevel + 1 < LEVEL_COUNT) StartLevel(currentLevel + 1);
        else                                GoToScene(Scene::Ending);
        break;
    case LevelAction::Retry:      StartLevel(currentLevel);   break;
    case LevelAction::BackToMenu: GoToScene(Scene::Menu);     break;
    case LevelAction::None:       break;
    }
}

void Game::DrawEnding()
{
    UI::DrawBackground(assets.endingBackground, Color{ 90, 60, 120, 255 }, Color{ 30, 20, 50, 255 });

    UI::DrawTextCentered("Ksiezniczka uratowana!", SCREEN_WIDTH / 2, 110, 56, GOLD);
    UI::DrawTextCentered("Dzieki tobie krolestwo znow jest bezpieczne.", SCREEN_WIDTH / 2, 195, 28, RAYWHITE);

    if (Button({ (SCREEN_WIDTH - 260) / 2.0f, 400, 260, 56 }, "Menu glowne"))
        GoToScene(Scene::Menu);
}
