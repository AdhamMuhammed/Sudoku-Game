#ifndef SudokuBoard_hpp
#define SudokuBoard_hpp
#include <vector>
#include <iosfwd>
/*
    File format
    -----------
    9 lines, 9 characters each. '0' and '.' both mean empty on read;
    '0' is what SaveToStream writes. No other characters are permitted.

    LoadFromStream throws std::invalid_argument on anything else: too few
    or too many lines, a short or long line.

    Givens that already break Sudoku rules (two 5s in a row) are loaded
    as-is. The board is storage, not a referee -- isSolved() will report
    false and the solver will find no completion.
*/


// The 9x9 grid and the rules of Sudoku. Knows about no other module.
// Coordinates are 0-based; out-of-range ones throw, rule violations return false.
class SudokuBoard{
    private:
        std::vector<std::vector<int>> board;
        bool isInRow(int row, int value)const;
        bool isInCol(int col, int value)const;
        bool isInBox(int row, int col, int value) const;
    public:
        // An empty board.
        SudokuBoard();

        // Builds a board from a 9x9 grid of givens, 0 meaning empty.
        // Throws std::invalid_argument if the shape is wrong or a value is
        // outside 0..9. Does not check Sudoku rules, so rule-breaking givens
        // (two 5s in a row) are accepted as-is.
        explicit SudokuBoard(const std::vector<std::vector<int>>& grid);

        // The value at row/col, 0 when empty. Throws std::out_of_range off-grid.
        int getCell(int row, int col) const;

        // Places value if the move is legal. Returns false and leaves the
        // board untouched if it is not.
        bool placeValue(int row, int col,int value);

        // Empties the cell. Clearing an already-empty cell is harmless.
        void clearCell(int row, int col);

        // True only when all 81 cells are filled and no rule is broken.
        bool isSolved() const;

        // Reads a new board in the format described above.
        // Throws std::invalid_argument on malformed input.
        static SudokuBoard LoadFromStream(std::istream& InputStream);

        // Writes the board in the format described above.
        // Returns false if the stream went bad.
        bool SaveToStream(std::ostream& OutputStream) const;


        // True if value may be placed at row/col. False if the cell is
        // already occupied, even by that same value.
        // Throws std::out_of_range for coordinates outside 0..8, and
        // std::invalid_argument for values outside 1..9 -- 0 is the empty
        // marker, not something a caller may place.
        bool isLegalMove(int row, int col, int value) const;
};

#endif