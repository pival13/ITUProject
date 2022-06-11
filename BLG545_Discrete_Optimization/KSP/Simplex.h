#pragma once

#include <vector>
#include <array>
#include <utility>
#include <iostream>
#include <cmath>

#include "Solver.h"

namespace LP {

class Simplex : Solver<double, double> {
    public:
        using typename Solver::Variable;
        using typename Solver::Variables;
        using typename Solver::ObjectifValue;
        using typename Solver::Result;
        using typename Solver::Results;
        using typename Solver::Equation;
        using typename Solver::Equations;
        using typename Solver::Output;

    public:
        Simplex(const Equation &objectivEq, const Equations &constrainsEq);
        virtual ~Simplex() {}

    public:
        virtual Output solve() override;

    private:
        std::vector<size_t> _vars;
        Equations _eqs;
};

static typename Simplex::Output solveSimplex(const typename Solver<double,double>::Equation &objectivEq, const typename Solver<double,double>::Equations &constrainsEq)
{
    Simplex s(objectivEq, constrainsEq);
    return s.solve();
}

};