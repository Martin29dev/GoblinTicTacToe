#include "Board.h"

namespace
{
    // 4 kierunki wystarczą (drugą stronę liczymy osobno): -, |, \, /
    const int DIRECTIONS[4][2] = { {0, 1}, {1, 0}, {1, 1}, {1, -1} };
}

Board::Board(int size, int winLength)
    : size(size), winLength(winLength), emptyCount(size * size),
      cells(size * size, Cell::Empty)
{
}

bool Board::Place(int index, Cell who)
{
    if (index < 0 || index >= CellCount()) return false;
    if (who == Cell::Empty || cells[index] != Cell::Empty) return false;

    cells[index] = who;
    emptyCount--;
    return true;
}

void Board::Remove(int index)
{
    if (index < 0 || index >= CellCount() || cells[index] == Cell::Empty) return;
    cells[index] = Cell::Empty;
    emptyCount++;
}

// Ile takich samych symboli jest w danym kierunku (bez pola startowego).
int Board::CountInDirection(int index, int dRow, int dCol) const
{
    const Cell who = cells[index];
    int row = index / size + dRow;
    int col = index % size + dCol;
    int count = 0;

    while (row >= 0 && row < size && col >= 0 && col < size && cells[row * size + col] == who)
    {
        count++;
        row += dRow;
        col += dCol;
    }
    return count;
}

bool Board::WinsAt(int index) const
{
    if (index < 0 || index >= CellCount() || cells[index] == Cell::Empty) return false;

    for (const auto& d : DIRECTIONS)
    {
        int total = 1 + CountInDirection(index, d[0], d[1])
                      + CountInDirection(index, -d[0], -d[1]);
        if (total >= winLength) return true;
    }
    return false;
}

std::vector<int> Board::WinningLine(int index) const
{
    std::vector<int> line;
    if (index < 0 || index >= CellCount() || cells[index] == Cell::Empty) return line;

    for (const auto& d : DIRECTIONS)
    {
        int forward  = CountInDirection(index, d[0], d[1]);
        int backward = CountInDirection(index, -d[0], -d[1]);
        if (1 + forward + backward < winLength) continue;

        int row = index / size;
        int col = index % size;
        for (int i = -backward; i <= forward; i++)
            line.push_back((row + i * d[0]) * size + (col + i * d[1]));
        break;
    }
    return line;
}

std::vector<int> Board::LegalMoves() const
{
    std::vector<int> moves;
    for (int i = 0; i < CellCount(); i++)
        if (cells[i] == Cell::Empty) moves.push_back(i);
    return moves;
}

Cell Board::Opponent(Cell who)
{
    return (who == Cell::Player) ? Cell::Enemy : Cell::Player;
}
