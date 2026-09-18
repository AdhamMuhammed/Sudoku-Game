/******************************************************************************
 *
 * File Name: SudokuSolver
 *
 * Author: Adham Muhammed
 *
 *******************************************************************************/
#include "SudokuSolver.hpp"

std::optional<Cell> SudokuSolver::findNextEmpty(const SudokuBoard& board){
    for(int i = 0;i<9;++i){
        for(int j = 0;j<9;++j){
            if(board.getCell(i,j) == 0){
                Cell EmptyCell;
                EmptyCell.row = i;
                EmptyCell.col = j;
                return EmptyCell;
            }
        }
    }
    return std::nullopt;
}

bool SudokuSolver::solveInPlace(SudokuBoard& board) const{
    std::optional<Cell> NextEmptyCell = findNextEmpty(board);
    if(!NextEmptyCell.has_value()){return board.isSolved();}
    for(int i = 1;i<10;i++){
        if(board.placeValue(NextEmptyCell.value().row,NextEmptyCell.value().col,i))
        {
            if(this->solveInPlace(board))
            {
                return true;
            }
            board.clearCell(NextEmptyCell.value().row,NextEmptyCell.value().col);
        }
    }
    return false;
    
}


std::optional<SudokuBoard> SudokuSolver::solve(SudokuBoard board) const{
    if(this->solveInPlace(board)){return board;}
    return std::nullopt;
}

int SudokuSolver::countSolutions(SudokuBoard board, int limit) const{
    if(limit<1){return 0;}
    return countInPlace(board,limit);
}

int SudokuSolver::countInPlace(SudokuBoard& board, int limit) const{
    std::optional<Cell> NextEmptyCell = findNextEmpty(board);
    if(!NextEmptyCell.has_value()){return (board.isSolved())?1 : 0;}
    int total = 0;
    for(int i = 1;i<10;i++){
        if(board.placeValue(NextEmptyCell.value().row,NextEmptyCell.value().col,i))
        {
            total+= countInPlace(board, limit-total);
            board.clearCell(NextEmptyCell.value().row,NextEmptyCell.value().col);
            if (total>=limit){return total;}
        }
    }
    return total;
    
}