# Sudoku

A console Sudoku game in C++17. Backtracking solver, puzzle file load and save,
optional puzzle generation with a uniqueness guarantee, and optional difficulty
rating based on human solving techniques.

The optional halves are genuine build options. The game compiles, links and runs
with either or both switched off.

## Features

- Full 9x9 board with rule checking on every placement
- Backtracking solver, plus solution counting with an early exit
- Move validation that goes beyond the rulebook: a placement that is legal but
  would leave the puzzle with no completion is rejected and undone
- Load and save puzzles as plain text
- Puzzle generation at three difficulties, every generated board guaranteed to
  have exactly one solution (optional, `BUILD_GENERATOR`)
- Difficulty rating by naked singles and naked pairs rather than clue count
  (optional, `BUILD_ADVANCED`)
- Data-driven menu, so a new entry can be registered from the composition root
  without touching the game loop

## Requirements

- A C++17 compiler. Developed against GCC 16.1 from MSYS2 UCRT64.
- CMake 3.8 or newer.

No third-party dependencies.

## Building

```sh
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

The generator argument is only needed where CMake cannot pick a toolchain on its
own. On Linux or macOS `cmake -S . -B build` is enough.

Two executables are produced:

```sh
./build/SudokuProject    # the game
./build/SudokuTests      # the test suite
```

### Build options

| Option | Default | Effect |
| --- | --- | --- |
| `BUILD_GENERATOR` | `ON` | Builds `SudokuGenerator` and adds the "Generate new puzzle" menu entry |
| `BUILD_ADVANCED` | `ON` | Builds `SudokuAdvancedChecks`, so generated puzzles are rated by technique instead of by clue count |

Option values are baked into a configured directory, so each combination needs
its own build tree:

```sh
cmake -S . -B build-off -G "MinGW Makefiles" -DBUILD_GENERATOR=OFF -DBUILD_ADVANCED=OFF
cmake --build build-off
```

All four combinations are expected to build and pass the test suite. With
`BUILD_GENERATOR=OFF` the generate entry simply does not appear. With
`BUILD_GENERATOR=ON` and `BUILD_ADVANCED=OFF` the generator falls back to hitting
a clue count per difficulty.

## Testing

The suite is a single assert-based executable with no test framework.

```sh
./build/SudokuTests
```

A clean run prints `All tests passed`. An assertion failure aborts on the spot,
so the absence of that line means something failed above it. There is no way to
run one case in isolation; comment out blocks in `tests/test_main.cpp` to narrow
things down.

## Playing

```
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
```

Rows and columns are 1 based at the prompt. A move is three numbers: row, column,
value.

`SamplePuzzle.txt` is a ready-made puzzle to load.

### A note on rejected moves

Two different rejections exist, and they mean different things:

- The move breaks a Sudoku rule, or the cell is already filled.
- The move breaks nothing, but no completion of the grid remains after it.

The second case is the interesting one. A puzzle with a unique solution has
exactly one correct digit per cell, and the rulebook cannot tell a wrong-but-legal
digit from the right one. Without this check a player discovers the mistake many
moves later, at a dead end, with no way to know where it happened. The move is
undone and the board is left exactly as it was.

## Puzzle file format

Nine lines of exactly nine characters. `0` and `.` both mean an empty cell on
read; `0` is what the game writes. Anything else is rejected.

```
530070000
600195000
098000060
800060003
400803001
700020006
060000280
000419005
000080079
```

Givens that already break the rules are loaded as they are rather than rejected.
The board is storage, not a referee, so such a puzzle loads, reports itself
unsolved, and yields no solution from the solver.

## Project layout

```
inc/      headers, one per module
src/      implementations, plus main.cpp
tests/    the assert-based test executable
```

| Module | Responsibility |
| --- | --- |
| `SudokuBoard` | The 9x9 grid, the rules, the file format. Depends on nothing else. |
| `SudokuSolver` | Backtracking `solve` and `countSolutions`. Depends on the board. |
| `SudokuGenerator` | Puzzle generation and clue removal. Optional. |
| `IDifficultyRater` | Rating interface. Header only, always built. |
| `SudokuAdvancedChecks` | Technique-based rating, the only rater implementation. Optional. |
| `SudokuGame` | Menu loop, rendering, input parsing, all console output. |
| `main.cpp` | Composition root. Constructs, wires, runs. |

Dependencies point one way only:

```
main.cpp -> SudokuGame -> SudokuSolver / SudokuGenerator -> SudokuBoard
```

Two rules hold this together and are worth preserving in any contribution:

- `SudokuGame` is the only module that reads `std::cin` or writes `std::cout`.
- `main.cpp` is the only module containing `#ifdef BUILD_GENERATOR` or
  `#ifdef BUILD_ADVANCED`. Conditional compilation anywhere else in `src/` or
  `inc/` defeats the point of the build options.

## Version 2.0

Two features are planned for the next release.

### Graphical interface

The console front end is already isolated. `SudokuGame` owns every stream access
in the project, and the board, solver and generator neither produce nor consume
text. A GUI is therefore a replacement for one module rather than a rewrite: the
same `SudokuBoard`, `SudokuSolver` and `SudokuGenerator` sit behind it unchanged.

Planned for this release:

- A grid widget with cell selection and keyboard entry
- Candidate marks shown per cell, driven by the same candidate logic the
  difficulty rater already computes
- Highlighting for the conflicting row, column or box when a move is refused
- A visible distinction between the two rejection kinds described above, which
  is easier to convey visually than in a line of text
- Difficulty selection and generation from a toolbar rather than a menu number

The menu is already a `std::vector` of label and callback pairs, so the actions a
GUI needs to bind to exist as callable units today.

### 16x16 boards

Hexadoku, a 16x16 grid of 4x4 boxes using sixteen symbols. The rules generalise
without change, but a fair amount of the code assumes nine:

- `SudokuBoard` needs its order and box size as parameters rather than literals,
  including the `/3*3` box-origin arithmetic
- The file format needs a symbol per cell that survives 16 values. Hexadecimal
  digits are the obvious choice and keep one character per cell, at the cost of
  no longer matching the 9x9 files byte for byte
- Rendering needs column widths that do not assume a single character per cell
- The solver's cost grows sharply. Plain backtracking that finishes instantly on
  9x9 is not guaranteed to, on 16x16, and constraint ordering such as choosing
  the most constrained cell first becomes worth the complexity
- Generation is the real bottleneck, since each candidate removal runs a
  uniqueness check. Expect this to need the solver work above before it is usable
- The rating techniques carry over unchanged in principle, but Easy and Hard mean
  different things on a larger grid and the thresholds need revisiting

Both are additive. Neither requires changing the rules layer, which is the part
of the project that has the most tests behind it.
