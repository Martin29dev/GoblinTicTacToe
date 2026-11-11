#pragma once
#include <vector>

// Co może leżeć na jednym polu planszy.
enum class Cell { Empty, Player, Enemy };

// Plansza size x size, wygrywa ten, kto ułoży winLength symboli w linii.
// Pola numerujemy od 0:  index = row * size + col
class Board
{
public:
    Board(int size = 3, int winLength = 3);

    int Size() const      { return size; }
    int WinLength() const { return winLength; }
    int CellCount() const { return size * size; }

    Cell Get(int index) const         { return cells[index]; }
    Cell Get(int row, int col) const  { return cells[row * size + col]; }

    bool Place(int index, Cell who);   // false = ruch niedozwolony
    void Remove(int index);            // cofnięcie ruchu (dla minimaxa)
    bool IsFull() const { return emptyCount == 0; }

    bool WinsAt(int index) const;                    // czy symbol na tym polu kończy linię
    std::vector<int> WinningLine(int index) const;   // pola wygrywającej linii (do podświetlenia)
    std::vector<int> LegalMoves() const;             // wszystkie wolne pola

    static Cell Opponent(Cell who);

private:
    int CountInDirection(int index, int dRow, int dCol) const;

    int size;
    int winLength;
    int emptyCount;
    std::vector<Cell> cells;
};
