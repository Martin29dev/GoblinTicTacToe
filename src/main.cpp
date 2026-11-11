#include <iostream>
#include "raylib.h"

using namespace std;

void Fade(float &alpha, float speed, bool fadingOut)
{
    // 1. Płynna zmiana wartości alfa
    if (fadingOut)
    {
        alpha += speed * GetFrameTime();
        if (alpha > 1.0f) alpha = 1.0f;
    }
    else
    {
        alpha -= speed * GetFrameTime();
        if (alpha < 0.0f) alpha = 0.0f;
    }

    // 2. Rysowanie nakładki na cały ekran, jeśli jest widoczna
    if (alpha > 0.0f)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, alpha));
    }
}

// Entry Point programu
int main(void)
{
    int storyCounter = 0;
    const int screenWidth = 960;
    const int screenHeight = 540;
    float backgroundTimer = 0.0f;
    bool backgroundChanged = false;

    InitWindow(screenWidth, screenHeight, "Goblin TicTacToe Game");

    Texture2D firstStoryBackground = LoadTexture("../assets/FirstStoryBackgroundWithGoblin.png");
    Texture2D secondStoryBackground = LoadTexture("../assets/SecondStoryBackgroundWithGoblin.png");
    Texture2D storyBackgrounds[10] = {
        LoadTexture("../assets/FirstStoryBackgroundWithGoblin.png"),
        LoadTexture("../assets/SecondStoryBackgroundWithGoblin.png"),
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
    };

    float alpha = 1.0f;
    bool fadingOut = false;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F))
        {
            ToggleBorderlessWindowed();
        }
        // 1. Wyzwolenie przejścia: po 2 sekundach LUB po wciśnięciu Enter
        if ((backgroundTimer >= 2.0f && !backgroundChanged) || IsKeyPressed(KEY_ENTER))
        {
            fadingOut = true; // Zaczynamy ściemniać ekran
        }

        // Jeśli obrazek jeszcze się nie zmienił i nie ściemniamy, licz czas
        if (!backgroundChanged && !fadingOut && alpha == 0.0f)
        {
            backgroundTimer += GetFrameTime();
        }

        // 2. Cała logika Fade Out -> Zmiana obrazka -> Automatyczny Fade In
        if (fadingOut)
        {
            // KROK A: Ściemnianie ekranu (Fade Out)
            alpha += 1.5f * GetFrameTime();
            if (alpha >= 1.0f)
            {
                alpha = 1.0f;
                
                // KROK B: Ekran jest czarny – zmieniamy obrazek (tylko raz!)
                if (!backgroundChanged)
                {
                    storyCounter++;
                    if (storyCounter > 1) storyCounter = 0; // Zapętlenie obrazków
                    backgroundChanged = true; // Blokujemy ponowne zmienianie
                }

                // KROK C: AUTOMATYCZNIE wyłączamy fadingOut, 
                // co natychmiast uruchamia rozjaśnianie (Fade In) bez klkania!
                fadingOut = false; 
            }
        }
        else
        {
            // Rozjaśnianie ekranu (Fade In) – dzieje się samoczynnie, aż alpha spadnie do 0
            if (alpha > 0.0f)
            {
                alpha -= 1.5f * GetFrameTime();
                if (alpha < 0.0f) alpha = 0.0f;
            }
        }
        BeginDrawing();

        // 1. Prostokąt źródłowy: bierzemy cały obrazek od (0,0) do jego pełnych wymiarów
        Rectangle sourceRec = { 0.0f, 0.0f, (float)storyBackgrounds[storyCounter].width, (float)storyBackgrounds[storyCounter].height };

        // 2. Prostokąt docelowy: dopasowujemy do aktualnego rozmiaru okna
        Rectangle destRec = { 0.0f, 0.0f, GetScreenWidth(), GetScreenHeight() };

        // 3. Punkt pochodzenia (środek transformacji) oraz rotacja
        Vector2 origin = { 0.0f, 0.0f };
        float rotation = 0.0f;

        // Rysowanie ze skalowaniem
        //DrawTexturePro(background, sourceRec, destRec, origin, rotation, WHITE);

            ClearBackground(RAYWHITE);
            //DrawTexture(background, 0, 0, WHITE); // origin x = 0 i y = 0
            DrawTexturePro(storyBackgrounds[storyCounter], sourceRec, destRec, origin, rotation, WHITE);
            DrawText("[Enter] pomin", GetScreenWidth() - 160, GetScreenHeight() - 40, 20, WHITE);
            Fade(alpha,1.5f, fadingOut);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}