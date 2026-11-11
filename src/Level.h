#pragma once
#include <memory>
#include <vector>
#include "AI.h"
#include "Assets.h"
#include "Board.h"
#include "Settings.h"

// Parametry jednego poziomu.
struct LevelConfig
{
    const char* title;
    int  boardSize;     // plansza boardSize x boardSize
    int  winLength;     // ile symboli w linii wygrywa
    int  aiDepth;       // 0 = strategia zachłanna, >0 = minimax alfa-beta o tej głębokości
    bool isFinal;       // czy to ostatni poziom
};

const LevelConfig& GetLevelConfig(int index);

enum class LevelResult { Playing, PlayerWon, EnemyWon, Draw };
enum class LevelAction { None, NextLevel, Retry, BackToMenu };

// Jeden ekran gry w kółko i krzyżyk (rycerz = X, goblin = O).
class LevelScreen
{
public:
    void Start(int levelIndex);
    LevelAction Update(const Assets& assets);
    void Draw(const Assets& assets, const Settings& settings) const;

private:
    bool PlayMove(int index, Cell who, const Assets& assets);
    void EnemyMove(const Assets& assets);
    int  CellAtMouse() const;
    Rectangle CellRect(int index) const;
    void DrawMark(int index, Cell who) const;
    void DrawEndOverlay(const Assets& assets) const;

    int levelIndex = 0;
    Board board;
    std::unique_ptr<Strategy> ai;

    bool  playerTurn = true;
    float enemyDelay = 0.0f;       // chwila "namysłu" goblina
    float endTimer   = 0.0f;       // czas od końca partii (opóźnia okno z wynikiem)
    LevelResult result = LevelResult::Playing;
    std::vector<int> winLine;

    bool    hasStats = false;
    AIStats lastStats;
};
