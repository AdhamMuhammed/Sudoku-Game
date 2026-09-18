#ifndef SudokuAdvancedChecks_hpp
#define SudokuAdvancedChecks_hpp
#include "IDifficultyRater.hpp"
#include <set>
#include <vector>
#include <utility>

/*
    Rates a puzzle like a person would, using increasingly powerful
    techniques and reporting the hardest one needed: Easy (naked singles
    alone), Medium (also naked pairs), Hard (both exhausted, cells still
    empty, needs guessing). No data members, so one instance can be shared
    and called as const.
*/
class SudokuAdvancedChecks : public IDifficultyRater {
public:
    // The 9x9 of remaining candidate digits per cell, owned by the rater, not the board.
    using CandidateGrid = std::vector<std::vector<std::set<int>>>;

    // Works on a copy of board; the caller's board is left untouched.
    Difficulty rate(const SudokuBoard& board) const override;

    // Digits that may legally go at (row, col), empty set if occupied. Throws std::out_of_range off-grid.
    std::set<int> candidates(const SudokuBoard& board, int row,int col)const;

    // Seeds all 81 candidate sets from the board; call only once, at rate's start.
    // Rebuilding mid-solve would silently erase eliminations, since the board can't record them.
    CandidateGrid buildCandidates(const SudokuBoard& board)const;

    // Erases a naked pair's two digits from the rest of its unit. Returns true only if something was actually erased.
    bool applyNakedPairs(CandidateGrid& grid) const;
private:
    // Places every cell with exactly one candidate left, updating the grid as it goes. Returns true only if something was placed.
    bool applyNakedSingles(CandidateGrid& grid, SudokuBoard& work) const;

    // Erases v from the candidate sets of (r,c)'s row, column and box peers.
    void eliminate(CandidateGrid& grid, int r, int c, int v)const;

    // The nine cells of unit k: 0-8 are rows, 9-17 columns, 18-26 boxes.
    static std::vector<std::pair<int,int>> unitCells(int k);

};


#endif
