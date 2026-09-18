#ifndef SudokuSolver_hpp
#define SudokuSolver_hpp
#include "SudokuBoard.hpp"
#include <optional>


// A row/column pair. The solver's own concept; SudokuBoard has no notion of it.
struct Cell
{
    int row;
    int col;
};

class SudokuSolver{
    private:
        // Backtracks to fill board in place. Returns whether a completion was found.
        bool solveInPlace(SudokuBoard& board) const;
        // Counts completions of board in place, stopping once total reaches limit.
        int countInPlace(SudokuBoard& board, int limit) const;
        // First empty cell in row-major order, nullopt if the board is full.
        static std::optional<Cell> findNextEmpty(const SudokuBoard& board);
    public:
        // Solves a copy of board, so the caller's board is never modified.
        // Deterministic and returns nullopt if unsolvable; an already-solved board comes back unchanged.
        std::optional<SudokuBoard> solve(SudokuBoard board) const;
        // Counts completions of a copy of board, stopping early once it reaches limit.
        // Returns 0 if none exist; pass limit 1 to just check a solution exists.
        int countSolutions(SudokuBoard board, int limit = 2) const;
};

#endif