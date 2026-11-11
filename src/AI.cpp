#include "AI.h"
#include <algorithm>
#include <chrono>
#include <numeric>
#include <random>

namespace
{
    constexpr int INF       = 1000000;
    constexpr int WIN_SCORE = 100000;
}

// ---------------------------------------------------------------- Strategy

int Strategy::ChooseMove(const Board& board, Cell me)
{
    stats = AIStats{};
    auto start = std::chrono::steady_clock::now();

    int move = FindMove(board, me);

    std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    stats.timeMs = elapsed.count();
    return move;
}

std::unique_ptr<Strategy> CreateStrategy(int depth)
{
    if (depth <= 0) return std::make_unique<GreedyStrategy>();
    return std::make_unique<MinimaxStrategy>(depth);
}

// ---------------------------------------------------------------- Greedy

std::string GreedyStrategy::Name() const { return "zachlanna"; }

int GreedyStrategy::FindMove(const Board& board, Cell me)
{
    Board work = board;
    const std::vector<int> moves = work.LegalMoves();
    if (moves.empty()) return -1;

    // 1. Wygraj od razu (me), 2. zablokuj wygraną przeciwnika.
    const Cell order[2] = { me, Board::Opponent(me) };
    for (Cell who : order)
    {
        for (int move : moves)
        {
            stats.nodes++;
            work.Place(move, who);
            bool wins = work.WinsAt(move);
            work.Remove(move);
            if (wins) return move;
        }
    }

    // 3. Zajmij środek.
    int center = (work.Size() / 2) * work.Size() + work.Size() / 2;
    if (work.Get(center) == Cell::Empty) return center;

    // 4. Losowe wolne pole.
    static std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<size_t> pick(0, moves.size() - 1);
    return moves[pick(rng)];
}

// ---------------------------------------------------------------- Minimax

MinimaxStrategy::MinimaxStrategy(int maxDepth) : maxDepth(maxDepth) {}

std::string MinimaxStrategy::Name() const
{
    return "minimax alfa-beta, glebokosc " + std::to_string(maxDepth);
}

void MinimaxStrategy::BuildMoveOrder(int boardSize)
{
    moveOrder.resize(boardSize * boardSize);
    std::iota(moveOrder.begin(), moveOrder.end(), 0);

    // Odległość od środka (współrzędne podwojone, żeby uniknąć liczb ułamkowych).
    auto distance = [boardSize](int index)
    {
        int r = 2 * (index / boardSize) - (boardSize - 1);
        int c = 2 * (index % boardSize) - (boardSize - 1);
        return r * r + c * c;
    };
    std::stable_sort(moveOrder.begin(), moveOrder.end(),
                     [&](int a, int b) { return distance(a) < distance(b); });
}

int MinimaxStrategy::FindMove(const Board& board, Cell me)
{
    Board work = board;
    aiCell = me;
    BuildMoveOrder(work.Size());

    int bestMove  = -1;
    int bestScore = -INF;
    int alpha     = -INF;

    for (int move : moveOrder)
    {
        if (work.Get(move) != Cell::Empty) continue;

        work.Place(move, me);
        int score = Search(work, maxDepth - 1, alpha, INF, false, move);
        work.Remove(move);

        if (score > bestScore)
        {
            bestScore = score;
            bestMove  = move;
        }
        alpha = std::max(alpha, bestScore);
    }
    return bestMove;
}

// aiTurn == true  -> teraz ruch AI (maksymalizujemy wynik)
// aiTurn == false -> teraz ruch gracza (minimalizujemy wynik)
// lastMove        -> pole, które zostało właśnie zajęte
int MinimaxStrategy::Search(Board& board, int depth, int alpha, int beta, bool aiTurn, int lastMove)
{
    stats.nodes++;

    // Ostatni ruch wykonał przeciwnik tego, kto jest teraz na ruchu.
    if (board.WinsAt(lastMove))
    {
        int score = WIN_SCORE + depth;          // szybsza wygrana = lepsza
        return aiTurn ? -score : score;
    }
    if (board.IsFull()) return 0;               // remis
    if (depth == 0) return Evaluate(board);     // koniec głębokości -> heurystyka

    const Cell who = aiTurn ? aiCell : Board::Opponent(aiCell);

    if (aiTurn)
    {
        int best = -INF;
        for (int move : moveOrder)
        {
            if (board.Get(move) != Cell::Empty) continue;

            board.Place(move, who);
            best = std::max(best, Search(board, depth - 1, alpha, beta, false, move));
            board.Remove(move);

            alpha = std::max(alpha, best);
            if (alpha >= beta) break;           // cięcie beta
        }
        return best;
    }
    else
    {
        int best = INF;
        for (int move : moveOrder)
        {
            if (board.Get(move) != Cell::Empty) continue;

            board.Place(move, who);
            best = std::min(best, Search(board, depth - 1, alpha, beta, true, move));
            board.Remove(move);

            beta = std::min(beta, best);
            if (alpha >= beta) break;           // cięcie alfa
        }
        return best;
    }
}

// Heurystyka dla niedokończonych układów: sprawdzamy każde "okno" długości winLength.
// Okno z samymi swoimi symbolami (i wolnymi polami) daje punkty, okno przeciwnika odejmuje.
int MinimaxStrategy::Evaluate(const Board& board) const
{
    static const int DIRS[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };
    const int n = board.Size();
    const int k = board.WinLength();
    int score = 0;

    for (int row = 0; row < n; row++)
    for (int col = 0; col < n; col++)
    for (const auto& d : DIRS)
    {
        int endRow = row + (k - 1) * d[0];
        int endCol = col + (k - 1) * d[1];
        if (endRow < 0 || endRow >= n || endCol < 0 || endCol >= n) continue;

        int mine = 0, theirs = 0;
        for (int i = 0; i < k; i++)
        {
            Cell c = board.Get(row + i * d[0], col + i * d[1]);
            if (c == aiCell) mine++;
            else if (c != Cell::Empty) theirs++;
        }

        if (theirs == 0 && mine > 0)   score += 1 << (3 * mine);    // 8, 64, 512, ...
        if (mine == 0 && theirs > 0)   score -= 1 << (3 * theirs);
    }
    return score;
}
