/******************************************************************************
 *
 * File Name: SudokuGenerator
 *
 * Author: Adham Muhammed
 *
 *******************************************************************************/
#include "SudokuGenerator.hpp"
#include <random>
#include <vector>
#include <algorithm>
#include <numeric>

SudokuBoard SudokuGenerator::dig(Difficulty diff)const{
    SudokuBoard MyBoard;
    std::vector<int> Digits{1,2,3,4,5,6,7,8,9};
    const int Target = diff == Difficulty::Easy ? 40 : diff == Difficulty::Medium ? 32 : 26;

    for(int i = 0;i<9;i+=3){
        std::shuffle(Digits.begin(),Digits.end(),Rng);
        for(int j = 0;j<3;j++){
            for(int k =0;k<3;k++){
                MyBoard.placeValue(i+j,i+k,Digits.at(j*3+k));
            }
        }
    }

    std::optional<SudokuBoard> SolvedBoard= Solver.solve(MyBoard);
    if(SolvedBoard.has_value())MyBoard = SolvedBoard.value();
    else{throw std::logic_error("SudokuGenerator: seeded diagonal boxes did not complete");}
    
    int Clues = 81;
    std::vector<int> Positions(81);
    std::iota(Positions.begin(),Positions.end(),0);
    std::shuffle(Positions.begin(),Positions.end(),Rng);

    for(int i = 0;i<81;i++){
        if(Clues == Target){break;}
        int Row = Positions.at(i) / 9;
        int Col = Positions.at(i) % 9;
        int Val = MyBoard.getCell(Row,Col);
        MyBoard.clearCell(Row,Col);
        if(Solver.countSolutions(MyBoard)!=1){MyBoard.placeValue(Row,Col,Val);} else Clues--;
    }

    return MyBoard;

}


SudokuBoard SudokuGenerator::generate(Difficulty diff)const{

    if(DifficultyRater == nullptr){return dig(diff);}


    SudokuBoard Board = dig(diff);
    for(int Attempt = 1; Attempt < MaxAttempts; ++Attempt){
        if(DifficultyRater->rate(Board) == diff){return Board;}
        Board = dig(diff);
    }

    return Board;
}
