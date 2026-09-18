#ifndef SudokuGame_hpp
#define SudokuGame_hpp
#include <string>
#include <functional>
#include <optional>
#include <SudokuBoard.hpp>
#include <SudokuSolver.hpp>

// One menu entry: the text shown and the action run when it is chosen.
struct MenuOption {
    std::string label;
    std::function<void()> action;
    MenuOption(std::string label, std::function<void()> action):label(label), action(action){}
};



// The only module that touches std::cout/std::cin. Owns the console menu
// loop and drives the board through the solver; holds no game rules itself.
class SudokuGame{
    private:
        const SudokuSolver& solver; // Used to solve and to check solution count on a move.
        SudokuBoard& board; // The board being played; replacing it changes what run() displays.
        std::vector<MenuOption> options; // The menu, in display order.
        bool isRunning; // False makes run() return after the current iteration.


        // Draws the 9x9 grid with box separators and '.' for empty cells.
        void printBoard();

        // Lists the options and reads a choice. Returns 0 and stops the game if input hit EOF.
        int printMenu();

        // Sets isRunning to false so run() exits after this iteration.
        void exit();
        // Prompts for row, column, value (1-9, converted to 0-based here) and places it.
        // Rejects a move that is rule-legal but would leave the puzzle unsolvable, even
        // though placeValue alone would have accepted it.
        void enterMove();
        // Replaces the board with the solver's solution, or reports that none exists.
        void solveAutomatically();
        // Prompts for a filename and loads it into the board. Leaves the board untouched on bad format or a missing file.
        void LoadFromFile();
        // Prompts for a filename and writes the current board to it.
        void SaveToFile();
    public:
        // Keeps a reference to board, so callers replacing it changes what the game shows.
        // Registers the built-in menu options (Enter a move, Solve, Load, Save, Exit).
        SudokuGame(SudokuBoard& board, const SudokuSolver& solver):board(board),solver(solver),isRunning(false){
            addOption("Exit",[this]{exit();});
            addOption("Enter a move",[this]{enterMove();});
            addOption("Solve automatically",[this]{solveAutomatically();});
            addOption("Load puzzle from file",[this]{LoadFromFile();});
            addOption("Save current puzzle to file",[this]{SaveToFile();});
        };
        // Runs the menu loop until the user exits or std::cin hits end of file.
        void run();
        // Adds a menu entry. Inserted before the last existing entry, so the
        // built-in Exit option stays pinned at the bottom of the menu.
        void addOption(std::string label, std::function<void()> func);

        // Prompts with label, re-prompting until the input is a number within [min, max].
        // Returns std::nullopt if the input stream hit end of file; callers should
        // then do nothing further, since the game is stopping.
        std::optional<int> promptForNumber(const std::string& label, int min, int max);

};
#endif