#ifndef SudokuGenerator_hpp
#define SudokuGenerator_hpp
#include "SudokuSolver.hpp"
#include "IDifficultyRater.hpp"
#include <random>
#include <ctime>

// Builds solved-then-dug puzzles with a unique solution. Holds a SudokuSolver&
// and an optional IDifficultyRater* without owning either, so both must outlive it.
class SudokuGenerator{
    private:
        const SudokuSolver& Solver;
        const IDifficultyRater* DifficultyRater;
        mutable std::mt19937 Rng;


        // Fills the diagonal boxes, solves the rest, then removes cells in shuffled
        // order, keeping a removal only if the board still has exactly one solution.
        // Stops at the clue count for diff (Easy 40, Medium 32, Hard 26).
        // Throws std::logic_error if the seeded diagonal boxes fail to complete,
        // which points to a bug rather than bad input.
        SudokuBoard dig(Difficulty diff) const;
    public:
        // Cap on puzzles generated and rated per generate() call when a rater is set.
        static constexpr int MaxAttempts = 10;

        // DifficultyRater may be null (the default), in which case diff is met by
        // clue count alone. seed defaults to the clock; pass a fixed value in tests
        // so a run is reproducible.
        SudokuGenerator(const SudokuSolver& Solver, const IDifficultyRater* DifficultyRater = nullptr, std::mt19937::result_type seed = time(nullptr)):
        Solver(Solver),DifficultyRater(DifficultyRater),Rng(seed){};

        // Always returns a board with exactly one solution; that is the only
        // guarantee. Without a rater diff is decided by clue count alone. With one,
        // up to MaxAttempts fresh puzzles are dug and rated (never dug further,
        // since removing clues is one-way) and the first rated diff is returned,
        // or the last attempt if none matched. Const, but Rng is mutable, so
        // repeated calls on the same object still produce different puzzles.
        SudokuBoard generate(Difficulty diff) const;
};

#endif