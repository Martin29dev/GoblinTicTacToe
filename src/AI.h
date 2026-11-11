#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Board.h"

// Statystyki ostatniego ruchu AI (do sprawozdania i do wyświetlenia w grze).
struct AIStats
{
    long long nodes  = 0;    // ile stanów gry odwiedzono
    double    timeMs = 0.0;  // ile trwało szukanie ruchu
};

// Wspólny interfejs strategii. Każda strategia implementuje tylko FindMove().
class Strategy
{
public:
    virtual ~Strategy() = default;

    // Wybiera ruch dla gracza "me" i mierzy czas.
    int ChooseMove(const Board& board, Cell me);

    const AIStats& GetStats() const { return stats; }
    virtual std::string Name() const = 0;

protected:
    virtual int FindMove(const Board& board, Cell me) = 0;
    AIStats stats;
};

// Strategia A: zachłanna. Wygraj, jeśli możesz -> zablokuj przeciwnika -> środek -> losowe pole.
class GreedyStrategy : public Strategy
{
public:
    std::string Name() const override;
protected:
    int FindMove(const Board& board, Cell me) override;
};

// Strategia B: Minimax z cięciami alfa-beta i ograniczoną głębokością.
class MinimaxStrategy : public Strategy
{
public:
    explicit MinimaxStrategy(int maxDepth);
    std::string Name() const override;
protected:
    int FindMove(const Board& board, Cell me) override;

private:
    int  Search(Board& board, int depth, int alpha, int beta, bool aiTurn, int lastMove);
    int  Evaluate(const Board& board) const;
    void BuildMoveOrder(int boardSize);

    int maxDepth;
    Cell aiCell = Cell::Enemy;
    std::vector<int> moveOrder;   // pola posortowane od środka (lepsze cięcia alfa-beta)
};

// depth == 0 -> strategia zachłanna, depth > 0 -> minimax o tej głębokości.
std::unique_ptr<Strategy> CreateStrategy(int depth);
