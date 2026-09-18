/******************************************************************************
 *
 * File Name: SudokuAdvancedChecks
 *
 * Author: Adham Muhammed
 *
 *******************************************************************************/

#include "SudokuAdvancedChecks.hpp"

bool SudokuAdvancedChecks::applyNakedSingles(CandidateGrid& grid, SudokuBoard& work) const{
    bool changed = false;
    for(int i = 0;i<9;i++){
        for(int j = 0;j < 9;j++){
            if(grid[i][j].size() == 1){
                int value = *grid[i][j].begin();
                if(work.placeValue(i,j,value)){
                    grid[i][j].clear();
                    changed = true;
                    eliminate(grid,i,j,value);
                }
            }
        }
    }
    return changed;
}

void SudokuAdvancedChecks::eliminate(CandidateGrid& grid, int r,int c,int v)const {
    for(int i = 0;i<9;i++){
        grid[r][i].erase(v);
        grid[i][c].erase(v);
    }
    int BoxRow = (r/3)*3;
    int BoxCol = (c/3)*3;
    for(int i = BoxRow;i<BoxRow+3;i++){
        for(int j = BoxCol;j<BoxCol+3;j++){
            grid[i][j].erase(v);
        }
    }
}

SudokuAdvancedChecks::CandidateGrid SudokuAdvancedChecks::buildCandidates(const SudokuBoard& board) const{
    CandidateGrid grid(9,std::vector<std::set<int>>(9));
    for(int i = 0;i<9;i++){
        for(int j = 0;j<9;j++){
            grid[i][j] = candidates(board,i,j);
        }
    }
    return grid;
}

std::set<int> SudokuAdvancedChecks::candidates(const SudokuBoard& board, int r,int c)const{
    std::set<int> result;
    for(int v =1;v<10;v++){
        if (board.isLegalMove(r,c,v))
        {
            result.insert(v);
        }
    }
    return result;
}


bool SudokuAdvancedChecks::applyNakedPairs(CandidateGrid& grid) const{
    bool changed = false;
    for(int k = 0;k<27;k++){
        std::vector<std::pair<int,int>> cells = unitCells(k);
        for(int a = 0;a<9;a++){
            std::set<int> Pair = grid[cells[a].first][cells[a].second];
            if(Pair.size()!=2){continue;}

            for(int b = a+1;b<9;b++){
                if(grid[cells[b].first][cells[b].second] != Pair){continue;}
                for(int x = 0;x<9;x++){
                    if(x==a || x==b){continue;}
                    std::set<int>& Other = grid[cells[x].first][cells[x].second];
                    for(int v : Pair){
                        if(Other.erase(v)>0){changed = true;}
                    }
                }
            }
        }
    }
    return changed;
}

std::vector<std::pair<int,int>> SudokuAdvancedChecks::unitCells(int k){
    std::vector<std::pair<int,int>> cells;
    cells.reserve(9);
    for(int i = 0;i<9;i++){
        if(k<9){
            cells.push_back({k,i});
        }
        else if(k<18){
            cells.push_back({i,k-9});
        }
        else{
            int b = k-18;
            cells.push_back({(b/3)*3 + i/3, (b%3)*3 + i%3});
        }
    }
    return cells;
}

Difficulty SudokuAdvancedChecks::rate(const SudokuBoard& board) const{
    SudokuBoard work = board;
    CandidateGrid grid = buildCandidates(work);
    Difficulty hardest = Difficulty::Easy;

    while(true){
        if(applyNakedSingles(grid,work)){continue;}

        if(applyNakedPairs(grid)){
            if(hardest == Difficulty::Easy){hardest = Difficulty::Medium;}
            continue;
        }

        break;
    }

    if(!work.isSolved()){return Difficulty::Hard;}
    return hardest;
}
