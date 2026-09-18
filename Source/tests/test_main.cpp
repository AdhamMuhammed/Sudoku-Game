/*****************************
Author : Adham Muhammed
******************************/
#include <iostream>
#include <cassert>
#include <stdexcept>
#include <sstream>
#include <optional>

#include "SudokuBoard.hpp"
#include "SudokuSolver.hpp"
#include "SudokuGame.hpp"
#include "IDifficultyRater.hpp"
#ifdef BUILD_GENERATOR
#include "SudokuGenerator.hpp"
#endif
#ifdef BUILD_ADVANCED
#include <set>
#include "SudokuAdvancedChecks.hpp"
#endif

#ifdef BUILD_GENERATOR
// Test doubles for IDifficultyRater. These are not a second concrete rater --
// SudokuAdvancedChecks is still the only one in the production chain. They
// exist so the generator's rater branch can be driven deterministically,
// including the path where no attempt ever matches.
class FixedRater : public IDifficultyRater {
public:
    explicit FixedRater(Difficulty level):Level(level){}
    Difficulty rate(const SudokuBoard&) const override { return Level; }
private:
    Difficulty Level;
};

class CountingRater : public IDifficultyRater {
public:
    explicit CountingRater(Difficulty level):Level(level){}
    Difficulty rate(const SudokuBoard&) const override { ++Calls; return Level; }
    mutable int Calls = 0;
private:
    Difficulty Level;
};
#endif

// ---------------------------------------------------------------------------
// Stream fixtures
// ---------------------------------------------------------------------------

// The sample puzzle in file format: 9 lines of 9 characters, 0 or . for empty.
const std::string kSampleText =
    "530070000\n"
    "600195000\n"
    "098...060\n"
    "800060003\n"
    "4..803001\n"
    "700020006\n"
    "060000280\n"
    "000419005\n"
    "000080079\n";

// 8 rows.
const std::string MalformedSample1 =
    "530070000\n"
    "600195000\n"
    "098000060\n"
    "800060003\n"
    "400803001\n"
    "700020006\n"
    "060000280\n"
    "000419005\n";

// Row 0 is 8 characters.
const std::string MalformedSample2 =
    "53007000\n"
    "600195000\n"
    "098000060\n"
    "800060003\n"
    "400803001\n"
    "700020006\n"
    "060000280\n"
    "000419005\n"
    "000080079\n";

// Non-digit 'q' in row 3. Mid-grid, so a loader that only checks row 0 fails.
const std::string MalformedSample3 =
    "530070000\n"
    "600195000\n"
    "098000060\n"
    "800q60003\n"
    "400803001\n"
    "700020006\n"
    "060000280\n"
    "000419005\n"
    "000080079\n";

// Row 4 is 10 characters.
const std::string MalformedSample4 =
    "530070000\n"
    "600195000\n"
    "098000060\n"
    "800060003\n"
    "4008030010\n"
    "700020006\n"
    "060000280\n"
    "000419005\n"
    "000080079\n";

// A valid grid followed by a tenth line -- the loader must not stop at nine.
const std::string MalformedSample5 =
    "530070000\n"
    "600195000\n"
    "098000060\n"
    "800060003\n"
    "400803001\n"
    "700020006\n"
    "060000280\n"
    "000419005\n"
    "000080079\n"
    "000000000\n";


// ---------------------------------------------------------------------------
// Board fixtures
// ---------------------------------------------------------------------------

// The sample puzzle from IO_Samples.md, 0 standing in for the '.' cells.
SudokuBoard makeSamplePuzzle(){
    return SudokuBoard({
        {5,3,0, 0,7,0, 0,0,0},
        {6,0,0, 1,9,5, 0,0,0},
        {0,9,8, 0,0,0, 0,6,0},

        {8,0,0, 0,6,0, 0,0,3},
        {4,0,0, 8,0,3, 0,0,1},
        {7,0,0, 0,2,0, 0,0,6},

        {0,6,0, 0,0,0, 2,8,0},
        {0,0,0, 4,1,9, 0,0,5},
        {0,0,0, 0,8,0, 0,7,9}
    });
}


// The unique solution to makeSamplePuzzle(). Worked out and checked separately
// -- section 6 of IO_Samples.md is deliberately incomplete.
SudokuBoard makeSolvedPuzzle(){
    return SudokuBoard({
        {5,3,4, 6,7,8, 9,1,2},
        {6,7,2, 1,9,5, 3,4,8},
        {1,9,8, 3,4,2, 5,6,7},

        {8,5,9, 7,6,1, 4,2,3},
        {4,2,6, 8,5,3, 7,9,1},
        {7,1,3, 9,2,4, 8,5,6},

        {9,6,1, 5,3,7, 2,8,4},
        {2,8,7, 4,1,9, 6,3,5},
        {3,4,5, 2,8,6, 1,7,9}
    });
}


// The solved grid with (0,8) holding 5 instead of 2, so row 0 has two 5s.
// All 81 cells are filled but the rules are broken -- this is what separates
// "isSolved checks the rules" from "isSolved counts filled cells".
SudokuBoard makeFilledInvalidPuzzle(){
    return SudokuBoard({
        {5,3,4, 6,7,8, 9,1,5},
        {6,7,2, 1,9,5, 3,4,8},
        {1,9,8, 3,4,2, 5,6,7},

        {8,5,9, 7,6,1, 4,2,3},
        {4,2,6, 8,5,3, 7,9,1},
        {7,1,3, 9,2,4, 8,5,6},

        {9,6,1, 5,3,7, 2,8,4},
        {2,8,7, 4,1,9, 6,3,5},
        {3,4,5, 2,8,6, 1,7,9}
    });
}


// The sample puzzle with a second 5 dropped into row 0 at (0,6). Still mostly
// empty, but the givens already break the rules, so no completion exists --
// this is the board solve() must answer nullopt for rather than throwing.
// The duplicate is the only violation: column 6 and box 2 stay clean.
SudokuBoard makeUnfilledInvalidPuzzle(){
    return SudokuBoard({
        {5,3,0, 0,7,0, 5,0,0},
        {6,0,0, 1,9,5, 0,0,0},
        {0,9,8, 0,0,0, 0,6,0},

        {8,0,0, 0,6,0, 0,0,3},
        {4,0,0, 8,0,3, 0,0,1},
        {7,0,0, 0,2,0, 0,0,6},

        {0,6,0, 0,0,0, 2,8,0},
        {0,0,0, 4,1,9, 0,0,5},
        {0,0,0, 0,8,0, 0,7,9}
    });
}


// The solved grid with the corners of a rectangle cleared: (5,3) and (7,5)
// held 9, (5,5) and (7,3) held 4. Rows 5 and 7, columns 3 and 5, and the
// four cells fall in exactly two boxes -- box (1,1) and box (2,1).
//
// Two boxes is the point. Four different boxes cannot work: a box holds all
// nine digits, so a box containing only one of the corners already holds the
// other value somewhere else, and the swap would duplicate it. With two cells
// per box the pair just trades places and every unit stays legal.
//
// So both the original arrangement and the 9/4 swap complete this grid, and
// nothing else does -- each corner's row, column and box are missing exactly
// {4, 9}. Count is 2.
SudokuBoard makeAmbiguousPuzzle(){
    return SudokuBoard({
        {5,3,4, 6,7,8, 9,1,2},
        {6,7,2, 1,9,5, 3,4,8},
        {1,9,8, 3,4,2, 5,6,7},

        {8,5,9, 7,6,1, 4,2,3},
        {4,2,6, 8,5,3, 7,9,1},
        {7,1,3, 0,2,0, 8,5,6},

        {9,6,1, 5,3,7, 2,8,4},
        {2,8,7, 0,1,0, 6,3,5},
        {3,4,5, 2,8,6, 1,7,9}
    });
}


#ifdef BUILD_ADVANCED

// A board holding a genuine naked pair, which the sample puzzle does not: every
// one of its 27 units was checked and none contains two cells sharing the same
// two candidates.
//
// Row 1 is the unit that matters. (1,2) and (1,8) are both exactly {2,8}, so
// between them they consume the 2 and the 8 of that row, and no other cell in
// row 1 may hold either digit. The row's only other empty cell is (1,1), which
// starts as {2,7,8} -- after the elimination it must be {7} alone, which is
// also a naked single. Unique solution, verified with countSolutions.
SudokuBoard makeNakedPairPuzzle(){
    return SudokuBoard({
        {5,3,4, 6,7,8, 9,1,0},
        {6,0,0, 1,9,5, 3,4,0},
        {1,0,0, 3,4,2, 5,6,7},

        {8,5,9, 7,6,1, 4,2,3},
        {4,0,6, 8,5,3, 7,9,1},
        {7,1,0, 9,2,4, 8,5,6},

        {9,0,1, 5,3,7, 2,8,4},
        {2,0,7, 4,1,9, 6,3,5},
        {3,4,5, 2,0,6, 1,7,9}
    });
}


// A 24-clue puzzle with exactly one solution that repeated naked-single passes
// cannot finish -- propagation stalls with cells still empty. The sample puzzle
// will not do for this: despite looking harder it *is* solvable by naked
// singles alone, so it rates Easy and cannot be the "requires more" case.
SudokuBoard makeBeyondNakedSinglesPuzzle(){
    return SudokuBoard({
        {0,0,0, 0,7,8, 9,1,0},
        {0,7,0, 0,0,5, 0,0,0},
        {1,9,0, 0,4,0, 0,6,0},

        {0,0,0, 7,0,0, 0,2,0},
        {0,0,6, 0,5,3, 0,0,0},
        {7,0,0, 9,0,0, 0,0,0},

        {0,0,0, 0,0,0, 0,8,0},
        {2,0,0, 0,0,0, 0,0,5},
        {0,4,5, 0,8,0, 1,0,0}
    });
}

#endif


int main(){

    // -----------------------------------------------------------------------
    // getCell
    // -----------------------------------------------------------------------

    {
        // A default board reads 0 everywhere. Also proves (0,0) and (8,8) --
        // the last valid indices -- do not throw.
        SudokuBoard board;
        for(int i = 0;i<9;++i){
            for(int j = 0; j<9;++j){
                assert(board.getCell(i,j)==0);
            }
        }
    }

    {
        // Each coordinate one step outside the grid throws.
        SudokuBoard board;
        bool threw = false;

        try{
            board.getCell(-1,0);
        }
        catch(const std::out_of_range&){
            threw = true;
        }
        assert(threw);

        threw = false;
        try{
            board.getCell(9,0);
        }
        catch(const std::out_of_range&){
            threw = true;
        }
        assert(threw);

        threw = false;
        try{
            board.getCell(0,9);
        }
        catch(const std::out_of_range&){
            threw = true;
        }
        assert(threw);

        threw = false;
        try{
            board.getCell(0,-1);
        }
        catch(const std::out_of_range&){
            threw = true;
        }
        assert(threw);
    }


    // -----------------------------------------------------------------------
    // isLegalMove
    // -----------------------------------------------------------------------

    {
        // The three conflict kinds, an occupied cell, and a known-good move.
        // Calling through a const reference proves the query does not mutate.
        SudokuBoard board = makeSamplePuzzle();
        const SudokuBoard ConstBoard = makeSamplePuzzle();

        assert(!board.isLegalMove(0,6,5));      //Row conflict
        assert(!board.isLegalMove(8,0,5));      //Col conflict
        assert(!board.isLegalMove(1,1,8));      //Box conflict
        assert(!board.isLegalMove(0,0,5));      //Occupied cell
        assert(board.isLegalMove(0,2,4));       //Legal move
        assert(ConstBoard.isLegalMove(0,2,4));  //Const board call
        assert(board.getCell(0,2) == 0);        //No mutation
    }

    {
        // Bad coordinates throw out_of_range.
        SudokuBoard board = makeSamplePuzzle();
        bool threw = false;

        try{
            board.isLegalMove(10,10,5);
        }
        catch(const std::out_of_range&){
            threw = true;
        }
        assert(threw);
    }

    {
        // Values outside 1-9 throw invalid_argument. 0 is the empty marker,
        // not a placeable value.
        SudokuBoard board = makeSamplePuzzle();
        bool threw = false;

        try{
            board.isLegalMove(3,3,0);
        }
        catch(const std::invalid_argument&){
            threw = true;
        }
        assert(threw);

        threw = false;
        try{
            board.isLegalMove(3,3,10);
        }
        catch(const std::invalid_argument&){
            threw = true;
        }
        assert(threw);

        threw = false;
        try{
            board.isLegalMove(3,3,-1);
        }
        catch(const std::invalid_argument&){
            threw = true;
        }
        assert(threw);
    }


    // -----------------------------------------------------------------------
    // placeValue and clearCell
    // -----------------------------------------------------------------------

    {
        // A legal move is applied; an illegal one leaves the cell untouched.
        SudokuBoard board = makeSamplePuzzle();

        assert(board.placeValue(0,2,4));
        assert(board.getCell(0,2) == 4);

        assert(!board.placeValue(2,0,6));
        assert(board.getCell(2,0) == 0);
    }

    {
        // Bad coordinates throw.
        SudokuBoard board = makeSamplePuzzle();
        bool threw = false;

        try{
            board.placeValue(10,2,4);
        }
        catch(const std::out_of_range&){
            threw = true;
        }
        assert(threw);
    }

    {
        // Clearing empties the cell; bad coordinates throw.
        SudokuBoard board = makeSamplePuzzle();
        bool threw = false;

        board.placeValue(0,2,4);
        board.clearCell(0,2);
        board.clearCell(0,8);
        assert(board.getCell(0,2) == 0);

        try{
            board.clearCell(10,2);
        }
        catch(const std::out_of_range&){
            threw = true;
        }
        assert(threw);
    }


    // -----------------------------------------------------------------------
    // isSolved
    // -----------------------------------------------------------------------

    {
        // Unfinished, filled-but-invalid, and genuinely solved.
        assert(!makeSamplePuzzle().isSolved());
        assert(!makeFilledInvalidPuzzle().isSolved());
        assert(makeSolvedPuzzle().isSolved());
    }


    // -----------------------------------------------------------------------
    // LoadFromStream
    // -----------------------------------------------------------------------

    {
        // A well-formed puzzle loads and spot-checked cells match.
        std::istringstream in(kSampleText);
        SudokuBoard ReadTest = SudokuBoard::LoadFromStream(in);

        assert(ReadTest.getCell(0,0) == 5);
        assert(ReadTest.getCell(0,2) == 0);
        assert(ReadTest.getCell(8,8) == 9);
    }

    {
        // Too few rows.
        std::istringstream in(MalformedSample1);
        bool threw = false;

        try{
            SudokuBoard::LoadFromStream(in);
        }
        catch(const std::invalid_argument&){
            threw = true;
        }
        assert(threw);
    }

    {
        // Short row.
        std::istringstream in(MalformedSample2);
        bool threw = false;

        try{
            SudokuBoard::LoadFromStream(in);
        }
        catch(const std::invalid_argument&){
            threw = true;
        }
        assert(threw);
    }

    {
        // Non-digit character.
        std::istringstream in(MalformedSample3);
        bool threw = false;

        try{
            SudokuBoard::LoadFromStream(in);
        }
        catch(const std::invalid_argument&){
            threw = true;
        }
        assert(threw);
    }

    {
        // Long row.
        std::istringstream in(MalformedSample4);
        bool threw = false;

        try{
            SudokuBoard::LoadFromStream(in);
        }
        catch(const std::invalid_argument&){
            threw = true;
        }
        assert(threw);
    }

    {
        // Too many rows.
        std::istringstream in(MalformedSample5);
        bool threw = false;

        try{
            SudokuBoard::LoadFromStream(in);
        }
        catch(const std::invalid_argument&){
            threw = true;
        }
        assert(threw);
    }

    {
        // Empty stream.
        std::istringstream in("");
        bool threw = false;

        try{
            SudokuBoard::LoadFromStream(in);
        }
        catch(const std::invalid_argument&){
            threw = true;
        }
        assert(threw);
    }

    {
        // Round trip: save to a string, read it back, compare all 81 cells.
        SudokuBoard Board = makeSamplePuzzle();

        std::ostringstream out;
        assert(Board.SaveToStream(out));

        std::istringstream in(out.str());
        SudokuBoard TestBoard = SudokuBoard::LoadFromStream(in);

        for(int i = 0; i<9;++i){
            for(int j = 0;j<9;++j){
                assert(Board.getCell(i,j)==TestBoard.getCell(i,j));
            }
        }
    }

    {
        //Invalid Board

        SudokuBoard InvalidBoard = makeFilledInvalidPuzzle();
        bool threw = false;

        std::ostringstream out;
        assert(InvalidBoard.SaveToStream(out));

        std::istringstream in(out.str());
        try{
            SudokuBoard Test = SudokuBoard::LoadFromStream(in);
            for(int i = 0; i<9;++i){
                for(int j = 0;j<9;++j){
                    assert(InvalidBoard.getCell(i,j)==Test.getCell(i,j));
                }
            }
        }
        catch(const std::exception&){
            threw = true;
        }
        assert(!threw);
    }



    // -------------------------------------------------------------------------
    // SudokuSolver tests
    // -------------------------------------------------------------------------

    {
        // The sample puzzle solves. The grid is checked against the known solution
        SudokuBoard TestBoard = makeSamplePuzzle();
        SudokuSolver TestSolver;

        std::optional<SudokuBoard> Result = TestSolver.solve(TestBoard);
        assert(Result.has_value());

        SudokuBoard SolvedBoard = Result.value();
        assert(SolvedBoard.isSolved());

        SudokuBoard Expected = makeSolvedPuzzle();
        for(int i = 0;i<9;++i){
            for(int j = 0;j<9;++j){
                assert(SolvedBoard.getCell(i,j) == Expected.getCell(i,j));
            }
        }

        assert(TestBoard.getCell(1,1)==0);
    }

    {
        // A board with nothing left to fill comes back exactly as it went in.
        SudokuBoard SolvedBoard = makeSolvedPuzzle();
        SudokuSolver TestSolver;

        std::optional<SudokuBoard> Result = TestSolver.solve(SolvedBoard);
        assert(Result.has_value());

        SudokuBoard TestBoard = Result.value();
        for(int i = 0;i<9;++i){
            for(int j = 0;j<9;++j){
                assert(TestBoard.getCell(i,j) == SolvedBoard.getCell(i,j));
            }
        }
    }

    {
        // A puzzle whose givens already conflict has no completion, and the
        // answer to that is nullopt, not an exception
        SudokuBoard InvalidPuzzle = makeUnfilledInvalidPuzzle();
        SudokuSolver TestSolver;
        std::optional<SudokuBoard> TestBoard = TestSolver.solve(InvalidPuzzle);
        assert(!TestBoard.has_value());
    }

    {
        //The test asserts that the result is *a* solved grid
        SudokuBoard EmptyBoard;
        SudokuSolver TestSolver;
        std::optional<SudokuBoard> TestBoard=TestSolver.solve(EmptyBoard);
        assert(TestBoard.has_value());
        assert(TestBoard.value().isSolved());
    }

    {
        // Determinism: the same input twice gives the same grid twice..
        SudokuBoard EmptyBoard1;
        SudokuBoard EmptyBoard2;
        SudokuSolver TestSolver;

        std::optional<SudokuBoard> TestBoard1 = TestSolver.solve(EmptyBoard1);
        std::optional<SudokuBoard> TestBoard2 = TestSolver.solve(EmptyBoard2);
        assert(TestBoard1.has_value());
        assert(TestBoard2.has_value());

        for(int i = 0;i<9;++i){
            for(int j = 0;j<9;++j){
                assert(TestBoard1.value().getCell(i,j) == TestBoard2.value().getCell(i,j));
            }
        }
    }


    {
        //Testing count solutions a solved board should have only one solutions
        SudokuBoard SolvedBoard = makeSolvedPuzzle();
        SudokuSolver TestSolver;
        assert(TestSolver.countSolutions(SolvedBoard)==1);
    }

    {
        //A proper puzzle should have only one possible solution
        SudokuBoard SampleBoard = makeSamplePuzzle();
        SudokuSolver TestSolver;
        assert(TestSolver.countSolutions(SampleBoard) == 1);
    }

    {
        //An invlaid board should have 0 possible solutions
        SudokuBoard InvalidBoard = makeFilledInvalidPuzzle();
        SudokuSolver TestSolver;
        assert(TestSolver.countSolutions(InvalidBoard)==0);
    }

    {
        // A puzzle with a genuine second answer counts 2, not 1. A solver that
        // stops at the first solution passes every test above this one and
        // fails here -- this is the test that separates counting from solving.
        SudokuBoard AmbiguousBoard = makeAmbiguousPuzzle();
        SudokuSolver TestSolver;
        assert(TestSolver.countSolutions(AmbiguousBoard) == 2);
    }

    {
        //An empty board takes forever to search the num of possible solutions so if it return when using limit 2 it means that it has abandoned the search
        SudokuBoard EmptyBoard;
        SudokuSolver TestSolver;

        assert(TestSolver.countSolutions(EmptyBoard,1) == 1);
        assert(TestSolver.countSolutions(EmptyBoard,2) == 2);
    }

    //{
    //     SudokuBoard SampleBoard = makeSamplePuzzle();
    //     const SudokuSolver Solver;
    //     SudokuGame Game(SampleBoard,Solver);
    //     Game.run();
    // }

    //Testing the generator
    #ifdef BUILD_GENERATOR
    {

        const SudokuSolver Solver;
        SudokuGenerator G1(Solver);
        assert(Solver.countSolutions(G1.generate(Difficulty::Easy)) == 1);
        assert(Solver.solve(G1.generate(Difficulty::Medium)).has_value());

    }

    {
        // Rater agrees on the first attempt, so generate returns immediately
        // and asks exactly once.
        const SudokuSolver Solver;
        const CountingRater Rater(Difficulty::Easy);
        SudokuGenerator G(Solver,&Rater,2024u);

        SudokuBoard Board = G.generate(Difficulty::Easy);
        assert(Rater.Calls == 1);
        assert(Solver.countSolutions(Board) == 1);
    }

    {
        // Rater never agrees. The loop must give up rather than run forever,
        // and the board it settles for must still be a real puzzle -- exactly
        // one solution is the property that survives a failed rating.
        const SudokuSolver Solver;
        const CountingRater Rater(Difficulty::Hard);
        SudokuGenerator G(Solver,&Rater,2024u);

        SudokuBoard Board = G.generate(Difficulty::Easy);
        assert(Rater.Calls > 1);
        assert(Rater.Calls == SudokuGenerator::MaxAttempts - 1);
        assert(Solver.countSolutions(Board) == 1);
    }

    {
        // A null rater must not consult anything, and must behave exactly as it
        // did before the rater branch existed: same seed, same puzzle.
        const SudokuSolver Solver;
        const FixedRater Rater(Difficulty::Easy);

        SudokuGenerator WithRater(Solver,&Rater,99u);
        SudokuGenerator WithoutRater(Solver,nullptr,99u);

        SudokuBoard A = WithRater.generate(Difficulty::Easy);
        SudokuBoard B = WithoutRater.generate(Difficulty::Easy);

        for(int i = 0;i<9;++i){
            for(int j = 0;j<9;++j){
                assert(A.getCell(i,j) == B.getCell(i,j));
            }
        }
    }
    #endif

    
    // SudokuAdvancedChecks
    

    #ifdef BUILD_ADVANCED

    {

        SudokuBoard Board = makeSamplePuzzle();
        const SudokuAdvancedChecks Rater;

        assert((Rater.candidates(Board,0,2) == std::set<int>{1,2,4}));
        assert((Rater.candidates(Board,0,3) == std::set<int>{2,6}));
        assert((Rater.candidates(Board,4,4) == std::set<int>{5}));
        assert(Rater.candidates(Board,0,0).empty());
    }

    {
        SudokuBoard Board = makeSamplePuzzle();
        const SudokuAdvancedChecks Rater;
        SudokuAdvancedChecks::CandidateGrid Grid = Rater.buildCandidates(Board);

        assert((Grid[4][4] == std::set<int>{5}));
        assert((Grid[6][5] == std::set<int>{7}));
        assert((Grid[6][8] == std::set<int>{4}));
        assert((Grid[7][7] == std::set<int>{3}));

        int Singles = 0;
        for(int i = 0;i<9;++i){
            for(int j = 0;j<9;++j){
                if(Grid[i][j].size() == 1){++Singles;}
            }
        }
        assert(Singles == 4);
    }

    {
        SudokuBoard Board = makeNakedPairPuzzle();
        const SudokuAdvancedChecks Rater;
        SudokuAdvancedChecks::CandidateGrid Grid = Rater.buildCandidates(Board);

        assert((Grid[1][2] == std::set<int>{2,8}));
        assert((Grid[1][8] == std::set<int>{2,8}));
        assert((Grid[1][1] == std::set<int>{2,7,8}));

        assert(Rater.applyNakedPairs(Grid));

        assert((Grid[1][1] == std::set<int>{7}));
        assert((Grid[1][2] == std::set<int>{2,8}));
        assert((Grid[1][8] == std::set<int>{2,8}));
    }

    {
        SudokuBoard Board = makeNakedPairPuzzle();
        const SudokuAdvancedChecks Rater;
        SudokuAdvancedChecks::CandidateGrid Grid = Rater.buildCandidates(Board);

        Rater.applyNakedPairs(Grid);
        assert(!Rater.applyNakedPairs(Grid));
    }

    {
        const SudokuAdvancedChecks Rater;

        assert(Rater.rate(makeSamplePuzzle()) == Difficulty::Easy);
        assert(Rater.rate(makeBeyondNakedSinglesPuzzle()) != Difficulty::Easy);
    }

    {
        SudokuBoard Board = makeSamplePuzzle();
        const SudokuAdvancedChecks Rater;

        Difficulty First = Rater.rate(Board);
        assert(Rater.rate(Board) == First);
        assert(Board.getCell(4,4) == 0);
    }



    #endif

    {
        SudokuBoard board = makeSamplePuzzle();
        SudokuSolver Solver;
        assert(Solver.countSolutions(board)>0);
        board.placeValue(0,2,4);
        assert(Solver.countSolutions(board)>0);

    }
    std::cout<<"All tests passed";


    return 0;
}