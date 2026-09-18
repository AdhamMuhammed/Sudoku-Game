/******************************************************************************
 *
 * File Name: SudokuGame
 *
 * Author: Adham Muhammed
 *
 *******************************************************************************/
#include "SudokuGame.hpp"
#include <iostream>
#include <fstream>
#include <limits>
void SudokuGame::run(){
    int choice;
    isRunning = true;
    while(isRunning){
        printBoard();
        try{
            choice = printMenu();
            if(!isRunning){break;}
            if(choice<1 || choice>options.size()){std::cout<<"Invalid choice"<<std::endl;continue;}
            options.at(choice-1).action();
        }
        catch(const std::invalid_argument&){
            std::cout<<"Input must be numeric"<<std::endl;
        }
    }
}

void SudokuGame::addOption(std::string label, std::function<void()> func){
    if(options.empty()){
        options.emplace_back(MenuOption(label,func));
    }else{
        options.insert(options.end()-1,MenuOption(label,func));
    }
}


void SudokuGame::exit(){isRunning = false;}

void SudokuGame::solveAutomatically(){
    std::optional<SudokuBoard> Solution = solver.solve(board);
    if(!Solution.has_value()){std::cout<<"Board has no valid solution!"<<std::endl;}
    else{
        board = Solution.value();
        std::cout<<"Puzzle Solved!"<<std::endl;
    }
}

void SudokuGame::LoadFromFile(){
    std::cout<<"Please enter filename: ";
    std::string FileName;
    std::cin>>FileName;
    std::ifstream inputFile(FileName);
    if(!inputFile.is_open()){std::cerr<<"Couldn't open the file"<<std::endl;return;}
    try{
        SudokuBoard temp = SudokuBoard::LoadFromStream(inputFile);
        board = temp;
        std::cout<<"Board successfully loaded!"<<std::endl;
    }
    catch(const std::invalid_argument&){
        std::cerr<<"Invalid file format"<<std::endl;
    }
    inputFile.close();
}
void SudokuGame::SaveToFile(){
    std::cout<<"Enter filename to save puzzle to:";
    std::string FileName;
    std::cin>>FileName;
    std::ofstream outputFile(FileName);
    if (!outputFile.is_open()){std::cerr<<"Unable to open file"<<std::endl;return;}
    if(!board.SaveToStream(outputFile)){std::cerr<<"Unable to write to file"<<std::endl;outputFile.close();return;}
    outputFile.close();
    std::cout<<"Puzzle saved to "<<FileName<<"!"<<std::endl;
}

void SudokuGame::printBoard(){
    int temp;
    std::cout<<"-------------------------------------"<<std::endl;
    for(int i = 0;i<9;++i){
        for(int j = 0; j<9;++j){
            temp = board.getCell(i,j);
            if(temp==0){std::cout<<". ";}
            else{std::cout<<temp<<" ";}
            if(j == 2||j == 5){std::cout<<"| ";}

        }
        std::cout<<std::endl;
        if(i == 2|| i==5||i==8)
        std::cout<<"-------------------------------------"<<std::endl;
    }
}

int SudokuGame::printMenu(){
    for(int i = 0;i<options.size();++i){
        std::cout<<i+1<<") "<<options[i].label<<std::endl;
    }
    std::cout<<"Choice: ";
    int temp;
    if(!(std::cin>>temp)){
        if(std::cin.eof()){isRunning = false; return 0;}
        std::cin.clear(); 
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        throw std::invalid_argument("Input must be an integer");
    }
    return temp;
}

void SudokuGame::enterMove(){
    std::cout<<"Enter row (1-9), column (1-9), and value (1-9): ";
    int row,col,val;
    while(!(std::cin>>row>>col>>val))
    {
        if(std::cin.eof()){isRunning = false; return;}
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout<<"Input must be numeric and press enter between row,col,val";
    }
    if(row>9 || row<1){ std::cout<<"Row must be between 1 and 9."<<std::endl; return;}
    if(col>9 || col<1){std::cout<<"Col must be between 1 and 9."<<std::endl;return;}
    if(val>9 || val<1){std::cout<<"Invalid value"<<std::endl;return;}
    if(board.placeValue(row-1,col-1,val)){
        if(solver.countSolutions(board,1)<1){board.clearCell(row-1,col-1);std::cout<<"Incorrect Move"<<std::endl;return;}
        if(board.isSolved()){std::cout<<"Great job! The Puzzle is solved!"<<std::endl;}
        else{std::cout<<"Move accepted!"<<std::endl;}
    }
    else{std::cout<<" Invalid move. That cell might be occupied or the placement breaks Sudoku rules."<<std::endl;}
}

std::optional<int> SudokuGame::promptForNumber(const std::string& label, int min, int max){
    while(true){
        std::cout<<label;
        int value;
        if(!(std::cin>>value)){
            if(std::cin.eof()){isRunning = false; return std::nullopt;}
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout<<"Input must be numeric"<<std::endl;
            continue;
        }
        if(value<min || value>max){
            std::cout<<"Please enter a number between "<<min<<" and "<<max<<"."<<std::endl;
            continue;
        }
        return value;
    }
}
