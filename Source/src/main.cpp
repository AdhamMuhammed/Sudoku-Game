/******************************************************************************
 *
 * File Name: main.cpp
 *
 * Author: Adham Muhammed
 *
 *******************************************************************************/

#include "SudokuBoard.hpp"
#include "SudokuGame.hpp"
#include "SudokuSolver.hpp"
#include <optional>
#ifdef BUILD_ADVANCED
#include "SudokuAdvancedChecks.hpp"
#endif

#ifdef BUILD_GENERATOR
#include "SudokuGenerator.hpp"
#endif

int main(){
    const SudokuSolver MySolver;
    SudokuBoard MyBoard;
    SudokuGame MyGame(MyBoard,MySolver);

#ifdef BUILD_ADVANCED
    const SudokuAdvancedChecks MyRater;
#endif

#ifdef BUILD_GENERATOR
  #ifdef BUILD_ADVANCED
    const SudokuGenerator MyGenerator(MySolver,&MyRater);
  #else
    const SudokuGenerator MyGenerator(MySolver);
  #endif
    MyGame.addOption("Generate new puzzle",[&]{
        std::optional<int> Level =
            MyGame.promptForNumber("Difficulty -- 1) Easy  2) Medium  3) Hard: ",1,3);
        if(!Level.has_value()){return;}

        Difficulty Requested = Level.value() == 1 ? Difficulty::Easy
                             : Level.value() == 2 ? Difficulty::Medium
                                                  : Difficulty::Hard;

        MyBoard = MyGenerator.generate(Requested);
    });
#endif

    MyGame.run();

    return 0;
}
