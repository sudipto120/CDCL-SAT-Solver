#include "solver.h"

// Clause methods implementation

bool Clause::isUnit(const std::unordered_map<int,bool> &assignment) const {
    int unassignedCount = 0;
    if(isTrue(assignment)) return false;
    for(const auto &lit:literals){
        auto it = assignment.find(lit.variable);
        if(it == assignment.end()) {
            unassignedCount++;
            if(unassignedCount>1) return false;
        }
    }
    return unassignedCount == 1;
}

bool Clause::isTrue(const std::unordered_map<int,bool> &assignment) const {
    for(const auto &lit:literals){
        auto it = assignment.find(lit.variable);
        if(it != assignment.end()){
            if((it->second && !lit.negated) || (!it->second && lit.negated)) return true;
        }
    }
    return false;
}

bool Clause::isFalse(const std::unordered_map<int,bool> &assignment) const {
    for(const auto &lit:literals){
        auto it = assignment.find(lit.variable);
        if(it == assignment.end()) return false;
        if((it->second && !lit.negated) || (!it->second && lit.negated)) return false;
    }
    return true;
}

Literal Clause::getUnitLiteral(const std::unordered_map<int,bool> &assignment) const {
    for(const auto &lit:literals){
        if(assignment.find(lit.variable) == assignment.end()) return lit;  
    }

    return Literal(0);  //Dummy return
}

//CDCLSolver Implementation

// Constructor
CDCLSolver::CDCLSolver() : decisionLevel(0) {}


// addClause
void CDCLSolver::addClause(const std::vector<Literal> &literals){
    auto clause = std::make_shared<Clause>(literals);
    clauses.push_back(clause);

    //Initialize the watch-list
    if(!literals.empty()){
        int lit1 = literals[0].getValue();
        watchList[lit1].push_back(clause);

        if(literals.size()>1){
            int lit2 = literals[1].getValue();
            watchList[lit2].push_back(clause);
        }
    }

    for(const auto &lit:literals) varActivity[lit.variable] = 0.0;

    return;
}


// Increment Activity 
void CDCLSolver::bumpVariableActivity(int variable){
    varActivity[variable] += 1.0;
}


Literal CDCLSolver::decide(){
    std::vector<int> unassigned;
    for(const auto &varAct : varActivity){
        if(assignment.find(varAct.first) == assignment.end()) unassigned.push_back(varAct.first);   
    }

    if(unassigned.empty()) return Literal(0); //Dummy return

    //VIDS
    int bestVar = unassigned[0];
    for(int i=1; i<unassigned.size(); ++i) if(varActivity[bestVar] < varActivity[unassigned[i]]) bestVar = unassigned[i];

    return Literal(bestVar);
}


void CDCLSolver::assign(Literal literal,int level, std::shared_ptr<Clause> reasonClause){
    int var = literal.variable;
    bool value = !literal.negated;

    assignment[var] = value;
    reason[var] = reasonClause;
    levelVars[level].insert(var);
    trail.emplace_back(literal,level,reasonClause);
    bumpVariableActivity(var);

    return;
}


std::shared_ptr<Clause> CDCLSolver::unitPropagation(){
    //unitPropagation without watch-list
    while(true){
        std::shared_ptr<Clause> unitClause = nullptr;

        for(const auto &clause:clauses){
            if(clause->isUnit(assignment)){
                unitClause = clause;
                break;
            }
        }

        if(unitClause == nullptr) return nullptr;

        Literal unitLit = unitClause->getUnitLiteral(assignment);
        assign(unitLit,decisionLevel,unitClause);

        auto conflict = checkConflict();
        if(conflict != nullptr) return conflict;
    }
    return nullptr;
}


std::shared_ptr<Clause> CDCLSolver::checkConflict(){
    for(auto &clause:clauses){
        if(clause->isFalse(assignment)) return clause;
    }
    return nullptr;
}


std::pair<std::shared_ptr<Clause>,int> CDCLSolver::analyzeConflict(std::shared_ptr<Clause> conflictClause){
    if(decisionLevel == 0) return {conflictClause,-1};

    std::unordered_set<Literal> conflictLiterals;
    auto &currentLevelVars = levelVars[decisionLevel];

    for(const auto &lit:conflictClause->literals){
        if(currentLevelVars.find(lit.variable) != currentLevelVars.end()) conflictLiterals.insert(lit);
    }

    auto currentClause = conflictClause;
    int backtrackLevel = 0;

    //Find the last set variable
    while(true){
        int lastVar = 0;
        for(const auto &lit:currentClause->literals){
            if(currentLevelVars.find(lit.variable) != currentLevelVars.end()){
                lastVar = lit.variable;
                break;
            }
        }

        if(lastVar == 0) break;

        auto reasonClause = reason[lastVar];
        if(reasonClause == nullptr) break;  //Decision Variable

        std::unordered_set<Literal> newLiterals;

        for(const auto &lit:currentClause->literals){
            if(lit.variable != lastVar) newLiterals.insert(lit);
        }
        for(const auto &lit:reasonClause->literals){
            if(lit.variable != lastVar) newLiterals.insert(lit);
        }

        std::vector<Literal> newLitVec(newLiterals.begin(),newLiterals.end());
        currentClause = std::make_shared<Clause>(newLitVec,true);

        int currentLvlCount = 0;
        for(const auto &lit:currentClause->literals){
            if(getDecisionLevel(lit.variable) == decisionLevel) currentLvlCount++;
        }

        if(currentLvlCount<=1) break;
    }

    //Calculate backtrack level
    for(const auto &lit:currentClause->literals){
        int varLevel = getDecisionLevel(lit.variable);
        if(varLevel > 0) backtrackLevel = std::max(backtrackLevel,varLevel);
    }

    return {currentClause,backtrackLevel};
}

int CDCLSolver::getDecisionLevel(int variable) const {
    for(auto it = trail.rbegin(); it != trail.rend(); ++it){
        if(it->literal.variable == variable) return it->level;
    }

    return -1; //Dummy return
}


void CDCLSolver::backtrack(int level){
    if(level<0) return;

    // Remove assignments upto backtrack level
    std::vector<int> varsToRemove;
    for(int dl = level; dl<=decisionLevel; ++dl){
        if(levelVars.find(dl) != levelVars.end()) {
            auto &vars = levelVars[dl];
            varsToRemove.insert(varsToRemove.end(),vars.begin(),vars.end());
        }
    }

    for(int var:varsToRemove){
        assignment.erase(var);
        reason.erase(var);
    }

    //Update trail
    auto newEnd = std::remove_if(trail.begin(),trail.end(),[level](const TrailEntry &entry){return entry.level>level;});
    trail.erase(newEnd,trail.end());
    decisionLevel = level;
}


bool CDCLSolver::solve(){
    //Unit propogation at level 0
    decisionLevel = 0;
    auto conflict = unitPropagation();

    if(conflict != nullptr) return false;

    while(true){
        Literal decision = decide();
        if(decision.variable == 0) return true; // All variables are assigned atp.

        decisionLevel++;

        assign(decision,decisionLevel,nullptr);

        while (true){
            conflict = unitPropagation();
            if(conflict == nullptr) break;
            
            //if there is some conflict
            if(decisionLevel == 0) return false;

            auto[learnedClause,backtrackLevel] = analyzeConflict(conflict);
            addClause(learnedClause->literals);
            clauses.back()->learned = true;

            backtrack(backtrackLevel);

            // backtrackLevel < 0
            if(backtrackLevel < decisionLevel){
                Literal assertLit(0);
                for(const auto&lit:learnedClause->literals){
                    if(assignment.find(lit.variable)==assignment.end()){
                        assertLit = lit;
                        break;
                    }
                }
                if(assertLit.variable != 0) assign(assertLit,backtrackLevel+1,learnedClause);
            }
        }
    }
}

void CDCLSolver::reset(){
    clauses.clear();
    assignment.clear();
    decisionLevel = 0;
    trail.clear();
    varActivity.clear();
    reason.clear();
    watchList.clear();
}

std::shared_ptr<CDCLSolver> CNFSolver(const std::vector<std::vector<int>>& cnfClauses) {
    auto solver = std::make_shared<CDCLSolver>();
    
    for (const auto& clause : cnfClauses) {
        std::vector<Literal> literals;
        for (int litVal : clause) {
            if (litVal == 0) continue;
            int variable = std::abs(litVal);
            bool negated = litVal < 0;
            literals.emplace_back(variable, negated);
        }
        
        if (!literals.empty()) {
            solver->addClause(literals);
        }
    }
    
    return solver;
}



