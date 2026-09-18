/******************************************************************************
 *
 * File Name: SudokuBoard
 *
 * Author: Adham Muhammed
 *
 *******************************************************************************/

#include "SudokuBoard.hpp"
#include <stdexcept>
#include <string>
#include <istream>
#include <ostream>
#include <cstdio>

SudokuBoard::SudokuBoard(){
    board.resize(9, std::vector<int>(9,0));
}

// Validates shape and the 0-9 range only. Sudoku rules are deliberately not
// checked here, so rule-breaking givens can be loaded from a file.
SudokuBoard::SudokuBoard(const std::vector<std::vector<int>>& grid){
    if(grid.size()!=9){
        throw std::invalid_argument("Grid must have exactly 9 rows");
    }
    for(const std::vector<int>& row : grid){
        if(row.size()!=9){
            throw std::invalid_argument("Every grid row must have exactly 9 columns");
        }
        for(int value : row){
            if(value<0 || value>9){
                throw std::invalid_argument("Grid values must be between 0 and 9");
            }
        }
    }
    board = grid;
}

int SudokuBoard::getCell(int row, int col) const{
    if(row>8 || row<0 || col>8 || col<0){
        throw std::out_of_range("Row and/or col out of range");
    }
    return board[row][col];
}

bool SudokuBoard::isInCol(int col,int value)const{
    for(int i = 0;i<9;++i){
        if (this->getCell(i,col) == value) return true;
    }
    return false;
}

bool SudokuBoard::isInRow(int row,int value)const{
    for(int i = 0;i<9;++i){
        if (this->getCell(row,i) == value) return true;
    }
    return false;
}

bool SudokuBoard::isInBox(int row, int col, int value)const{
    //Top-left corner of the containing box
    int RowOrigin = (row/3)*3;
    int ColOrigin = (col/3)*3;
    for(int i = 0;i<3;++i){
        for(int j = 0;j<3;++j){
            if(this->getCell(RowOrigin+i,ColOrigin+j)==value) return true;
        }
    }
    return false;
}

// Arguments are validated before any lookup, so a bad call throws rather
// than reporting an illegal move.
bool SudokuBoard::isLegalMove(int row, int col, int value) const{
    if(row<0 || row>8 || col<0 ||col>8){throw std::out_of_range("Row and/or col out of range");} //Bad coordinate check
    if(value<1 || value>9){throw std::invalid_argument("Value should be between 1-9");} //Bad value check
    if(this->getCell(row,col)!=0){return false;}    //Occupied cell
    if(this->isInCol(col,value)){return false;}    //col conflict
    if(this->isInRow(row,value)){return false;}    //row conflict
    if(this->isInBox(row,col,value)){return false;}
    return true;
}

// The rules live in isLegalMove; this only writes once they allow it.
bool SudokuBoard::placeValue(int row, int col, int value){
    if(isLegalMove(row,col,value))
    {
        board[row][col] = value;
        return true;
    }
    return false;
}

void SudokuBoard::clearCell(int row,int col){
    if(row<0 || row>8 || col<0 || col>8){
        throw std::out_of_range("Row and/or col out of range");
    }
    board[row][col] = 0;
}

// Checks the rules rather than counting filled cells -- a full grid holding a
// duplicate is not solved. Every row, column and box must hold 1-9.
bool SudokuBoard::isSolved() const{
    for(int i =0;i<9;++i){
        for(int j = 1;j<10;++j){
            if(!isInRow(i,j) || !isInCol(i,j)){return false;}
        }
    }
    for(int r = 0; r < 9; r += 3)
    {
        for(int c = 0; c < 9; c += 3)
        {
            for(int v = 1; v <= 9; ++v)
            {
                if(!isInBox(r, c, v))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

// Builds into a local grid and hands it to the validating constructor.
// placeValue is not used, so conflicting givens survive the load.
SudokuBoard SudokuBoard::LoadFromStream(std::istream& InputStream){
    if(InputStream.peek() == EOF){throw std::invalid_argument("Input stream is empty");}
    std::vector<std::vector<int>> TempBoard;
    std::string temp;

    while(std::getline(InputStream, temp)){
        if(temp.length()!=9){
            throw std::invalid_argument("Row length should be 9 char long");
        }
        std::vector<int> row;
        for(int i = 0;i<9;++i){
            if(temp[i] == '.' || temp[i] == '0'){
                row.push_back(0);
            }
            else if(temp[i] >= '1' && temp[i] <= '9'){
                row.push_back(temp[i] - '0');
            }
            else{
                throw std::invalid_argument("Characters should be 0-9 or .");
            }

        }
        TempBoard.push_back(row);
    }
    //Catches too few rows and too many rows
    if(TempBoard.size()!=9){throw std::invalid_argument("There must be only 9 rows");}
    return SudokuBoard(TempBoard);
}


// One line per row, '0' for empty
bool SudokuBoard::SaveToStream(std::ostream& OutputStream) const{

    for(int i = 0;i<9;++i){
        for(int j = 0;j<9;++j){
            OutputStream << static_cast<char>('0' + this->getCell(i,j));
        }
        OutputStream << '\n';
    }
    return static_cast<bool>(OutputStream);

}