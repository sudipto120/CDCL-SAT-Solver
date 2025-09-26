#pragma once


#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <memory>
#include <algorithm>


class Literal{
public:
    int variable;
    bool negated;

    Literal(int var = 0, bool neg = false) : variable(var), negated(neg) {}
    
    int getValue() const {
        return negated ? -variable:variable;
    }

    Literal operator-() const {
        return Literal(variable,!negated);
    }

    bool operator==(const Literal &other) const {
        return (variable == other.variable) && (negated == other.negated);
    }
};


namespace std {
    template <>
    struct hash<Literal> {
        size_t operator()(const Literal& lit) const {
            return hash<int>()(lit.getValue());
        }
    };
}





class Clause{
public:
    std::vector<Literal> literals;
    bool learned;
    double activity;

    Clause(const std::vector<Literal>& lits, bool learn = false) 
        : literals(lits), learned(learn), activity(0.0) {}

    bool isUnit(const std::unordered_map<int, bool>& assignment) const;
    bool isTrue(const std::unordered_map<int, bool>& assignment) const;
    bool isFalse(const std::unordered_map<int, bool>& assignment) const;
    Literal getUnitLiteral(const std::unordered_map<int,bool>& assignment) const;
};





class CDCLSolver{
private:
    std::vector<std::shared_ptr<Clause>> clauses;
    std::unordered_map<int, bool> assignment;
    int decisionLevel;

    struct TrailEntry {
        Literal literal;
        int level;
        std::shared_ptr<Clause> reason;
        TrailEntry(Literal lit, int lvl, std::shared_ptr<Clause> r) 
            : literal(lit), level(lvl), reason(r) {}
    };
    
    std::vector<TrailEntry> trail;
    std::unordered_map<int, double> varActivity;
    std::unordered_map<int, std::shared_ptr<Clause>> reason;
    std::unordered_map<int, std::unordered_set<int>> levelVars;
    std::unordered_map<int, std::vector<std::shared_ptr<Clause>>> watchList;
    
    // Core CDCL methods
    Literal decide();
    std::shared_ptr<Clause> unitPropagation();
    void assign(Literal literal, int level, std::shared_ptr<Clause> reasonClause = nullptr);
    std::shared_ptr<Clause> checkConflict();
    std::pair<std::shared_ptr<Clause>, int> analyzeConflict(std::shared_ptr<Clause> conflictClause);
    void backtrack(int level);
    int getDecisionLevel(int variable) const;
    void bumpVariableActivity(int variable);
    
public:
    CDCLSolver();
    void addClause(const std::vector<Literal>& literals);
    bool solve();
    std::unordered_map<int, bool> getAssignment() const { return assignment; }
    void reset();

};

std::shared_ptr<CDCLSolver> CNFSolver(const std::vector<std::vector<int>>& cnfClauses);