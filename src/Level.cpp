#include "Level.h"
#include <cmath>
#include "Config.h"
#include "UI.h"

namespace
{
    // Plansza zawsze ma 360x360 px, niezależnie od liczby pól.
    const float BOARD_PIXELS = 360.0f;
    const Rectangle BOARD_RECT = { (SCREEN_WIDTH - BOARD_PIXELS) / 2, 95.0f, BOARD_PIXELS, BOARD_PIXELS };

    const float END_OVERLAY_DELAY = 0.8f;
    const float ENEMY_THINK_TIME  = 0.6f;

    // Okno z wynikiem partii.
    const Rectangle PANEL_RECT   = { 280.0f, 170.0f, 400.0f, 210.0f };
    const Rectangle BUTTON_FIRST = { 300.0f, 305.0f, 170.0f, 50.0f };
    const Rectangle BUTTON_MENU  = { 490.0f, 305.0f, 170.0f, 50.0f };

    const Color PLAYER_COLOR = { 90, 170, 255, 255 };
    const Color ENEMY_COLOR  = { 110, 220, 90, 255 };
}

// Tu zmieniasz trudność poziomów.
const LevelConfig& GetLevelConfig(int index)
{
    static const LevelConfig levels[LEVEL_COUNT] = {
        //  tytuł                          plansza  wygrana  głębokość  ostatni
        { "Poziom 1 - Lesna sciezka",         3,       3,       0,      false },
        { "Poziom 2 - Mroczna jaskinia",      4,       3,       3,      false },
        { "Poziom 3 - Twierdza goblina",      5,       4,       5,      true  },
    };
    return levels[index];
}

// ---------------------------------------------------------------- logika

void LevelScreen::Start(int index)
{
    const LevelConfig& config = GetLevelConfig(index);

    levelIndex = index;
    board = Board(config.boardSize, config.winLength);
    ai = CreateStrategy(config.aiDepth);

    playerTurn = true;
    enemyDelay = 0.0f;
    endTimer   = 0.0f;
    result     = LevelResult::Playing;
    winLine.clear();
    hasStats = false;
}

// Wykonuje ruch i sprawdza koniec gry. Zwraca false dla ruchu niedozwolonego.
bool LevelScreen::PlayMove(int index, Cell who, const Assets& assets)
{
    if (!board.Place(index, who)) return false;

    if (board.WinsAt(index))
    {
        winLine = board.WinningLine(index);
        if (who == Cell::Player) { result = LevelResult::PlayerWon; assets.win.Play(); }
        else                     { result = LevelResult::EnemyWon;  assets.lose.Play(); }
    }
    else if (board.IsFull())
    {
        result = LevelResult::Draw;
    }
    else
    {
        playerTurn = (who == Cell::Enemy);
        enemyDelay = ENEMY_THINK_TIME;
    }
    return true;
}

void LevelScreen::EnemyMove(const Assets& assets)
{
    int move = ai->ChooseMove(board, Cell::Enemy);
    lastStats = ai->GetStats();
    hasStats = true;
    PlayMove(move, Cell::Enemy, assets);
}

Rectangle LevelScreen::CellRect(int index) const
{
    const int n = board.Size();
    const float cellSize = BOARD_PIXELS / n;
    return { BOARD_RECT.x + (index % n) * cellSize,
             BOARD_RECT.y + (index / n) * cellSize,
             cellSize, cellSize };
}

// Zwraca numer pola pod myszką albo -1.
int LevelScreen::CellAtMouse() const
{
    Vector2 mouse = GetMousePosition();
    if (!CheckCollisionPointRec(mouse, BOARD_RECT)) return -1;

    const int n = board.Size();
    const float cellSize = BOARD_PIXELS / n;
    int col = (int)((mouse.x - BOARD_RECT.x) / cellSize);
    int row = (int)((mouse.y - BOARD_RECT.y) / cellSize);
    if (col >= n) col = n - 1;
    if (row >= n) row = n - 1;
    return row * n + col;
}

LevelAction LevelScreen::Update(const Assets& assets)
{
    const float dt = GetFrameTime();

    if (result == LevelResult::Playing)
    {
        if (playerTurn)
        {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                int index = CellAtMouse();
                if (index >= 0 && PlayMove(index, Cell::Player, assets))
                    assets.click.Play();
            }
        }
        else
        {
            enemyDelay -= dt;
            if (enemyDelay <= 0.0f) EnemyMove(assets);
        }
        return LevelAction::None;
    }

    // Partia skończona - po chwili obsługujemy przyciski okna z wynikiem.
    endTimer += dt;
    if (endTimer < END_OVERLAY_DELAY) return LevelAction::None;

    if (UI::IsClicked(BUTTON_FIRST))
    {
        assets.click.Play();
        return (result == LevelResult::PlayerWon) ? LevelAction::NextLevel : LevelAction::Retry;
    }
    if (UI::IsClicked(BUTTON_MENU))
    {
        assets.click.Play();
        return LevelAction::BackToMenu;
    }
    return LevelAction::None;
}

// ---------------------------------------------------------------- rysowanie

void LevelScreen::DrawMark(int index, Cell who) const
{
    Rectangle cell = CellRect(index);
    float pad = cell.width * 0.22f;
    float thick = cell.width * 0.09f;

    if (who == Cell::Player)   // rycerz = X
    {
        DrawLineEx({ cell.x + pad, cell.y + pad }, { cell.x + cell.width - pad, cell.y + cell.height - pad }, thick, PLAYER_COLOR);
        DrawLineEx({ cell.x + cell.width - pad, cell.y + pad }, { cell.x + pad, cell.y + cell.height - pad }, thick, PLAYER_COLOR);
    }
    else                       // goblin = O
    {
        Vector2 center = { cell.x + cell.width / 2, cell.y + cell.height / 2 };
        float outer = cell.width / 2 - pad;
        DrawRing(center, outer - thick, outer, 0.0f, 360.0f, 40, ENEMY_COLOR);
    }
}

void LevelScreen::Draw(const Assets& assets, const Settings& settings) const
{
    const LevelConfig& config = GetLevelConfig(levelIndex);
    const bool playing = (result == LevelResult::Playing);

    UI::DrawBackground(assets.levelBackgrounds[levelIndex], Color{ 40, 70, 45, 255 }, Color{ 15, 25, 20, 255 });

    // --- postacie: goblin w prawym górnym rogu, rycerz w lewym dolnym ---
    // Postać, która ma teraz ruch, delikatnie podskakuje.
    float bob = sinf((float)GetTime() * 5.0f) * 4.0f;
    float goblinBob = (playing && !playerTurn) ? bob : 0.0f;
    float knightBob = (playing && playerTurn)  ? bob : 0.0f;

    Rectangle goblinArea = { SCREEN_WIDTH - 170.0f, 10.0f + goblinBob, 160.0f, 160.0f };
    Rectangle knightArea = { 10.0f, SCREEN_HEIGHT - 170.0f + knightBob, 160.0f, 160.0f };
    UI::DrawTextureContain(assets.goblin, goblinArea, "Goblin", DARKGREEN);
    UI::DrawTextureContain(assets.knight, knightArea, "Rycerz", DARKBLUE);

    // --- nagłówek ---
    UI::DrawTextCentered(config.title, SCREEN_WIDTH / 2, 14, 30, GOLD);
    UI::DrawTextCentered(TextFormat("Plansza %dx%d  |  wygrywa %d w linii", config.boardSize, config.boardSize, config.winLength),
                         SCREEN_WIDTH / 2, 52, 20, RAYWHITE);

    // --- plansza ---
    DrawRectangleRec(BOARD_RECT, Fade(BLACK, 0.45f));

    const int n = board.Size();
    const float cellSize = BOARD_PIXELS / n;
    for (int i = 1; i < n; i++)
    {
        float offset = i * cellSize;
        DrawLineEx({ BOARD_RECT.x + offset, BOARD_RECT.y }, { BOARD_RECT.x + offset, BOARD_RECT.y + BOARD_PIXELS }, 4.0f, RAYWHITE);
        DrawLineEx({ BOARD_RECT.x, BOARD_RECT.y + offset }, { BOARD_RECT.x + BOARD_PIXELS, BOARD_RECT.y + offset }, 4.0f, RAYWHITE);
    }

    if (playing && playerTurn)   // podświetlenie pola pod myszką
    {
        int hover = CellAtMouse();
        if (hover >= 0 && board.Get(hover) == Cell::Empty)
            DrawRectangleRec(CellRect(hover), Fade(WHITE, 0.2f));
    }

    for (int index : winLine)    // wygrywająca linia
        DrawRectangleRec(CellRect(index), Fade(GOLD, 0.35f));

    for (int i = 0; i < board.CellCount(); i++)
        if (board.Get(i) != Cell::Empty) DrawMark(i, board.Get(i));

    // --- status i statystyki AI ---
    if (playing)
        UI::DrawTextCentered(playerTurn ? "Twoj ruch, rycerzu!" : "Goblin mysli...", SCREEN_WIDTH / 2, 466, 24, RAYWHITE);

    if (settings.showStats && hasStats)
    {
        UI::DrawTextCentered(TextFormat("AI: %s | wezly: %lld | czas: %.2f ms",
                             ai->Name().c_str(), lastStats.nodes, lastStats.timeMs),
                             SCREEN_WIDTH / 2, 504, 18, LIGHTGRAY);
    }

    if (!playing && endTimer >= END_OVERLAY_DELAY) DrawEndOverlay(assets);
}

void LevelScreen::DrawEndOverlay(const Assets& assets) const
{
    (void)assets;
    const LevelConfig& config = GetLevelConfig(levelIndex);

    const char* title = "";
    const char* subtitle = "";
    const char* firstButton = "Jeszcze raz";

    if (result == LevelResult::PlayerWon)
    {
        title = "Zwyciestwo!";
        subtitle = config.isFinal ? "Droga do ksiezniczki wolna!" : "Goblin pokonany. Idziemy dalej!";
        firstButton = config.isFinal ? "Do ksiezniczki" : "Dalej";
    }
    else if (result == LevelResult::EnemyWon)
    {
        title = "Porazka...";
        subtitle = "Goblin sie smieje. Sprobuj jeszcze raz!";
    }
    else
    {
        title = "Remis!";
        subtitle = "Nikt nie wygral. Sprobuj ponownie.";
    }

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.55f));
    DrawRectangleRounded(PANEL_RECT, 0.1f, 8, Color{ 40, 30, 25, 245 });

    UI::DrawTextCentered(title, SCREEN_WIDTH / 2, (int)PANEL_RECT.y + 30, 40, GOLD);
    UI::DrawTextCentered(subtitle, SCREEN_WIDTH / 2, (int)PANEL_RECT.y + 90, 20, RAYWHITE);

    UI::DrawButton(BUTTON_FIRST, firstButton);
    UI::DrawButton(BUTTON_MENU, "Menu");
}
