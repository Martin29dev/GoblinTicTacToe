#pragma once
#include "raylib.h"

// Proste narzędzia do rysowania interfejsu.
namespace UI
{
    void SetEnabled(bool enabled);                 // false = klikanie wyłączone (np. podczas przejścia)

    bool IsHovered(Rectangle rect);
    bool IsClicked(Rectangle rect);

    void DrawButton(Rectangle rect, const char* text);      // tylko rysuje
    bool Button(Rectangle rect, const char* text);          // rysuje + zwraca true po kliknięciu

    void DrawTextCentered(const char* text, int centerX, int y, int fontSize, Color color);

    // Rysuje obrazek wpisany w prostokąt (zachowuje proporcje). Gdy brak pliku - kolorowy zastępnik.
    void DrawTextureContain(Texture2D tex, Rectangle area, const char* fallbackLabel, Color fallbackColor);

    // Tło na cały ekran. Gdy brak obrazka - gradient.
    void DrawBackground(Texture2D tex, Color gradientTop, Color gradientBottom);
}
