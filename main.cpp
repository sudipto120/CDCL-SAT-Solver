#include "solver.h"
#include <fstream>
#include <cmath>

std::vector<std::vector<int>> solve_sudoku(std::vector<std::vector<int>> grid){

    int n = grid.size();
    int sn = std::sqrt(n);

    auto f = [n](int i, int j, int k){
        return (n * n) * (k - 1) + n * j + i + 1;
    };

    auto invf = [n](int x) {
        x = x - 1;
        int i = x % n;
        int j = (x / n) % n;
        int k = x / (n * n) + 1;
        return std::make_tuple(i, j, k);
    };

    std::vector<std::vector<int>> CNFClauses;

    //Initial Conditions
    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            if (grid[i][j] != 0) CNFClauses.push_back({f(i,j,grid[i][j])});  
        }
    }

    // Each cell can have at most one number
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k1 = 1; k1 <= n; k1++) {
                for (int k2 = k1 + 1; k2 <= n; k2++) {
                    CNFClauses.push_back({-f(i, j, k1), -f(i, j, k2)});
                }
            }
        }
    }

    // Each number appears exactly once in each row
    for (int k = 1; k <= n; k++) {
        for (int i = 0; i < n; i++) {
            // At least once in the row
            std::vector<int> clause;
            for (int j = 0; j < n; j++) {
                clause.push_back(f(i, j, k));
            }
            CNFClauses.push_back(clause);

            // At most once in the row
            for (int j1 = 0; j1 < n; j1++) {
                for (int j2 = j1 + 1; j2 < n; j2++) {
                    CNFClauses.push_back({-f(i, j1, k), -f(i, j2, k)});
                }
            }
        }
    }

    for (int k = 1; k <= n; k++) {

        // Each value in a row
        for (int i = 0; i < n; i++) {
            std::vector<int> row_clause;
            for (int j = 0; j < n; j++) row_clause.push_back(f(i, j, k));
            CNFClauses.push_back(row_clause);
        }

        // Each value in a column
        for (int j = 0; j < n; j++) {
            std::vector<int> col_clause;
            for (int i = 0; i < n; i++) col_clause.push_back(f(i, j, k));
            CNFClauses.push_back(col_clause);
        }

        // Each value in a square
        for (int i_ = 0; i_ < sn; i_++) {
            for (int j_ = 0; j_ < sn; j_++) {
                std::vector<int> sq_clause;
                for (int i = sn * i_; i < sn * (i_ + 1); i++) {
                    for (int j = sn * j_; j < sn * (j_ + 1); j++) {
                        sq_clause.push_back(f(i, j, k));
                    }
                }
                CNFClauses.push_back(sq_clause);
            }
        }
}

    auto Solver = CNFSolver(CNFClauses);

    bool valid = Solver->solve();
    
    if(valid){
        auto assignment = Solver->getAssignment();
        std::vector<std::vector<int>> solutionGrid(n,std::vector<int>(n,0));
        for(auto lit:assignment){
            if(lit.second && lit.first > 0){
                auto [i,j,k] = invf(lit.first);
                solutionGrid[i][j] = k;
            }
        }
        return solutionGrid;
    }
    
    else return {};

}


bool isValidSudoku(const std::vector<std::vector<int>>& board) {

    int n = board.size();
    int sn = std::sqrt(n);


    // Check rows
    for (int i = 0; i < n; i++) {
        std::unordered_set<int> seen;
        for (int j = 0; j < n; j++) {
            int num = board[i][j];
            if (num < 1 || num > n || seen.count(num)) return false;
            seen.insert(num);
        }
    }

    // Check columns
    for (int j = 0; j < n; j++) {
        std::unordered_set<int> seen;
        for (int i = 0; i < n; i++) {
            int num = board[i][j];
            if (num < 1 || num > n || seen.count(num)) return false;
            seen.insert(num);
        }
    }

    // Check subgrids of size sn x sn
    for (int br = 0; br < n; br += sn) {
        for (int bc = 0; bc < n; bc += sn) {
            std::unordered_set<int> seen;
            for (int i = 0; i < sn; i++) {
                for (int j = 0; j < sn; j++) {
                    int num = board[br + i][bc + j];
                    if (num < 1 || num > n || seen.count(num)) return false;
                    seen.insert(num);
                }
            }
        }
    }

    return true;
}

int main() {

    std::vector<std::vector<int>> samplePuzzle = {
        {5, 3, 0, 0, 7, 0, 0, 0, 0},
        {6, 0, 0, 1, 9, 5, 0, 0, 0},
        {0, 9, 8, 0, 0, 0, 0, 6, 0},
        {8, 0, 0, 0, 6, 0, 0, 0, 3},
        {4, 0, 0, 8, 0, 3, 0, 0, 1},
        {7, 0, 0, 0, 2, 0, 0, 0, 6},
        {0, 6, 0, 0, 0, 0, 2, 8, 0},
        {0, 0, 0, 4, 1, 9, 0, 0, 5},
        {0, 0, 0, 0, 8, 0, 0, 7, 9}
    };

    auto solution = solve_sudoku(samplePuzzle);

    for(auto row:solution){
        for(auto val:row) std::cout<<val<<" ";  
        std::cout<<std::endl;
    }


    return 0;
}