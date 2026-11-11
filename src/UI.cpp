#include "UI.h"
#include <algorithm>
#include "Config.h"

namespace UI
{
    static bool inputEnabled = true;

    void SetEnabled(bool enabled) { inputEnabled = enabled; }

    bool IsHovered(Rectangle rect)
    {
        return inputEnabled && CheckCollisionPointRec(GetMousePosition(), rect);
    }

    bool IsClicked(Rectangle rect)
    {
        return IsHovered(rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    }

    void DrawButton(Rectangle rect, const char* text)
    {
        const bool hovered = IsHovered(rect);
        const Color fill = hovered ? Color{ 190, 120, 40, 255 } : Color{ 80, 50, 30, 255 };
        const int fontSize = 28;

        DrawRectangleRounded({ rect.x - 3, rect.y - 3, rect.width + 6, rect.height + 6 }, 0.3f, 8, GOLD);
        DrawRectangleRounded(rect, 0.3f, 8, fill);

        int textX = (int)(rect.x + rect.width / 2) - MeasureText(text, fontSize) / 2;
        int textY = (int)(rect.y + (rect.height - fontSize) / 2);
        DrawText(text, textX, textY, fontSize, RAYWHITE);
    }

    bool Button(Rectangle rect, const char* text)
    {
        DrawButton(rect, text);
        return IsClicked(rect);
    }

    void DrawTextCentered(const char* text, int centerX, int y, int fontSize, Color color)
    {
        int x = centerX - MeasureText(text, fontSize) / 2;
        DrawText(text, x + 2, y + 2, fontSize, Color{ 0, 0, 0, 160 });   // cień
        DrawText(text, x, y, fontSize, color);
    }

    void DrawTextureContain(Texture2D tex, Rectangle area, const char* fallbackLabel, Color fallbackColor)
    {
        if (tex.id == 0)
        {
            DrawRectangleRounded(area, 0.2f, 8, fallbackColor);
            DrawTextCentered(fallbackLabel, (int)(area.x + area.width / 2), (int)(area.y + area.height / 2 - 12), 24, RAYWHITE);
            return;
        }

        float scale = std::min(area.width / tex.width, area.height / tex.height);
        Rectangle source = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
        Rectangle dest = {
            area.x + (area.width  - tex.width  * scale) / 2,
            area.y + (area.height - tex.height * scale) / 2,
            tex.width * scale,
            tex.height * scale
        };
        DrawTexturePro(tex, source, dest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
    }

    void DrawBackground(Texture2D tex, Color gradientTop, Color gradientBottom)
    {
        if (tex.id == 0)
        {
            DrawRectangleGradientV(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, gradientTop, gradientBottom);
            return;
        }
        Rectangle source = { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
        Rectangle dest   = { 0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT };
        DrawTexturePro(tex, source, dest, Vector2{ 0.0f, 0.0f }, 0.0f, WHITE);
    }
}
