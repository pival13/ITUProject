#pragma once

#include "Simplex.h"
#include <list>

namespace BIP {

class BranchAndBound : Solver<double, bool> {
    public:
        using typename Solver::Variable;
        using typename Solver::Variables;
        using typename Solver::ObjectifValue;
        using typename Solver::Result;
        using typename Solver::Results;
        using typename Solver::Equation;
        using typename Solver::Equations;
        using typename Solver::Output;

    private:
        enum Value : int8_t {
            Undefined = -1,
            False = 0,
            True
        };
        using State = std::vector<Value>;
    
    public:
        BranchAndBound(const Equation &objectivEq, const Equations &constrainsEq);
        virtual ~BranchAndBound() {}

    public:
        virtual Output solve() override;

    private:
        std::tuple<LP::Simplex::Equation, LP::Simplex::Equations> generateFunctions(const State &state) const;
        void updateStates(std::list<std::tuple<ObjectifValue, State>> &states) const;
        std::list<State> getNewState(const State &refState, const LP::Simplex::Results &setValues) const;

    private:
        const Equation _objEq;
        const Equations _constEqs;

        ObjectifValue optimalRes = -HUGE_VAL;
        Results optimalValues;
};

static typename Solver<double,bool>::Output solveBranchBound(const typename Solver<double,bool>::Equation &objectivEq, const typename Solver<double,bool>::Equations &constrainsEq)
{
    BranchAndBound bb(objectivEq, constrainsEq);
    return bb.solve();
}

}