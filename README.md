# Sudoku

A console Sudoku game written in C++17 featuring a backtracking solver, puzzle file loading and saving, optional puzzle generation with a uniqueness guarantee, and optional difficulty rating based on human solving techniques.

The optional features are genuine build options. The project can compile, link, and run with either or both features disabled.

## Features

- Full 9x9 Sudoku board with rule checking on every placement
- Backtracking solver with solution counting and early exit
- Move validation that rejects legal placements that would leave the puzzle unsolvable
- Load and save puzzles using plain-text files
- Puzzle generation with three difficulty levels
- Guaranteed unique solution for every generated puzzle
- Optional difficulty rating using naked singles and naked pairs
- Data-driven menu architecture using registered label/callback pairs
- Unit testing using a lightweight assert-based test suite
- CMake build system
- No third-party dependencies

## Requirements

- C++17 compiler
- CMake 3.8 or newer
- Developed and tested with GCC 16.1 using MSYS2 UCRT64
- No third-party dependencies

## Building

### Windows / MinGW

    cmake -S . -B build -G "MinGW Makefiles"
    cmake --build build

### Linux / macOS

    cmake -S . -B build
    cmake --build build

The generator argument is only required when CMake cannot automatically select the appropriate toolchain.

Two executables are produced:

    ./build/SudokuProject
    ./build/SudokuTests

`SudokuProject` is the game executable and `SudokuTests` is the test suite.

## Build Options

| Option | Default | Description |
|---|---|---|
| `BUILD_GENERATOR` | `ON` | Builds `SudokuGenerator` and enables the "Generate new puzzle" menu entry |
| `BUILD_ADVANCED` | `ON` | Builds `SudokuAdvancedChecks` for technique-based difficulty rating |

Each configuration should use its own build directory.

Example with both optional features disabled:

    cmake -S . -B build-off -G "MinGW Makefiles" -DBUILD_GENERATOR=OFF -DBUILD_ADVANCED=OFF
    cmake --build build-off

All four combinations of the two options are expected to build and pass the test suite.

When `BUILD_GENERATOR=OFF`, the puzzle generation menu entry is removed.

When `BUILD_GENERATOR=ON` and `BUILD_ADVANCED=OFF`, generated puzzles fall back to difficulty classification based on clue count.

## Testing

The project includes a single assert-based test executable without an external testing framework.

Run:

    ./build/SudokuTests

A successful test run prints:

    All tests passed

Assertion failures abort execution at the failing test.

## Playing

The game provides a console-based menu:

    -------------------------------------

    5 3 . | . 7 . | . . .
    6 . . | 1 9 5 | . . .
    . 9 8 | . . . | . 6 .

    -------------------------------------

    8 . . | . 6 . | . . 3
    4 . . | 8 . 3 | . . 1
    7 . . | . 2 . | . . 6

    -------------------------------------

    . 6 . | . . . | 2 8 .
    . . . | 4 1 9 | . . 5
    . . . | . 8 . | . 7 9

    -------------------------------------

    1) Enter a move
    2) Solve automatically
    3) Load puzzle from file
    4) Save current puzzle to file
    5) Generate new puzzle
    6) Exit

    Choice:

Rows and columns are 1-based at the prompt.

A move is entered using three numbers:

    row column value

For example:

    3 4 7

`SamplePuzzle.txt` contains a ready-made puzzle for testing the load functionality.

## Move Validation

The game performs two levels of validation.

### Rule Violation

A move is rejected if:

- The selected cell is already filled
- The value violates the Sudoku row rule
- The value violates the Sudoku column rule
- The value violates the Sudoku box rule

### Unsolvable Placement

A move can be legal according to the Sudoku rules while still making the entire puzzle impossible to complete.

The game detects this case by checking whether a valid completion still exists after the placement.

If no solution remains:

- The move is rejected
- The placement is undone
- The board remains unchanged

This allows the player to discover an invalid logical choice immediately instead of reaching a dead end much later.

## Puzzle File Format

Puzzle files contain exactly nine lines with nine characters per line.

Both `0` and `.` represent empty cells when reading a puzzle.

The game writes empty cells as `0`.

Example:

    530070000
    600195000
    098000060
    800060003
    400803001
    700020006
    060000280
    000419005
    000080079

Characters other than `0` and `.` are rejected.

Puzzles containing initial rule violations are loaded as provided. The board acts as storage rather than a referee, so such puzzles can be loaded but will be reported as unsolved and the solver will find no solution.

## Project Architecture

    main.cpp -> SudokuGame -> SudokuSolver / SudokuGenerator -> SudokuBoard

Dependencies are intentionally kept in one direction.

### SudokuBoard

Responsible for:

- 9x9 grid storage
- Sudoku rule checking
- Cell manipulation
- Puzzle file format
- Board state

The board does not depend on other project modules.

### SudokuSolver

Responsible for:

- Backtracking solving
- Solution counting
- Early termination when enough solutions are found

Depends on `SudokuBoard`.

### SudokuGenerator

Responsible for:

- Generating new puzzles
- Removing clues
- Maintaining the unique-solution guarantee

Optional through `BUILD_GENERATOR`.

### IDifficultyRater

Defines the difficulty-rating interface.

The interface is always available.

### SudokuAdvancedChecks

Provides technique-based difficulty rating using:

- Naked singles
- Naked pairs

Optional through `BUILD_ADVANCED`.

### SudokuGame

Responsible for:

- Console menu
- Board rendering
- User input
- Input parsing
- Game flow
- Console output

`SudokuGame` is the only module that reads from `std::cin` or writes to `std::cout`.

### main.cpp

Acts as the composition root.

It constructs and wires the required components and starts the game.

`main.cpp` is also the only module containing the `BUILD_GENERATOR` and `BUILD_ADVANCED` conditional compilation logic.

## Project Structure

    Sudoku-Game/
    │
    ├── CMakeLists.txt
    ├── README.md
    ├── SamplePuzzle.txt
    │
    ├── inc/
    │   └── Project headers
    │
    ├── src/
    │   ├── main.cpp
    │   └── Project implementations
    │
    └── tests/
        └── test_main.cpp

## Design Principles

Two architectural rules are intentionally preserved throughout the project:

1. `SudokuGame` is the only module responsible for console input and output.
2. `main.cpp` is the only module containing conditional compilation for optional features.

This keeps the core Sudoku logic independent from the user interface and build configuration.

## Technologies & Concepts

- C++17
- Object-Oriented Programming
- Encapsulation
- Abstraction
- Interfaces
- Callbacks
- Composition
- Modular Design
- Dependency Direction
- Backtracking Algorithms
- Constraint Checking
- Solution Counting
- Puzzle Generation
- Unique-Solution Validation
- CMake
- Unit Testing
- Conditional Compilation
- File I/O
- Console Application Design

## Planned Version 2.0

### Graphical Interface

The console interface is isolated inside `SudokuGame`, while the board, solver, and generator remain independent from console input and output.

This allows a future graphical interface to reuse the existing Sudoku logic.

Planned features include:

- Interactive Sudoku grid
- Cell selection and keyboard input
- Candidate marks
- Conflict highlighting
- Visual distinction between rule violations and unsolvable moves
- Difficulty selection
- Puzzle generation controls

### 16x16 Boards

A future version may support 16x16 Hexadoku boards using 4x4 subgrids and sixteen symbols.

This would require:

- Parameterizing board size and box size
- Updating the file format
- Supporting sixteen cell symbols
- Updating board rendering
- Improving solver constraint ordering
- Optimizing puzzle generation
- Revisiting difficulty thresholds

The existing architecture is designed so these additions can be developed without replacing the core project structure.

## Author

**Adham Muhammed**
## Project Type

Educational C++ OOP project demonstrating object-oriented design, modular architecture, backtracking algorithms, puzzle generation, file handling, build configuration, and unit testing.
