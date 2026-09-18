#ifndef IDifficultyRater_hpp
#define IDifficultyRater_hpp
#include "SudokuBoard.hpp"

// A rated puzzle difficulty.
enum class Difficulty{Easy, Medium, Hard};

// Header-only rating interface, built in every configuration. SudokuAdvancedChecks
// is the only implementation and lives behind the BUILD_ADVANCED flag.
class IDifficultyRater{
    public:
    virtual ~IDifficultyRater() = default;

    // Judges how hard board is to solve. Const, so an implementation that needs
    // to place digits must work on its own copy of board, not this one.
    virtual Difficulty rate(const SudokuBoard& board) const = 0;
};

#endif