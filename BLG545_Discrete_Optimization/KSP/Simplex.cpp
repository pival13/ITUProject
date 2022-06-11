#include "Simplex.h"
#include <algorithm>

namespace LP {

Simplex::Simplex(const Equation &obj, const Equations &constrs)
{
    const size_t EquNb = constrs.size();
    const size_t VarNb = obj.coeffs.size();

    _vars.resize(EquNb + 1);
    _eqs.resize(EquNb + 1);

    {
        _vars.front() = -1;
        _eqs.front().result = obj.result;
        _eqs.front().coeffs.resize(VarNb + EquNb);
        auto itEqCoeff = _eqs.front().coeffs.begin();
        for (auto itObjCoeff = obj.coeffs.begin(); itObjCoeff != obj.coeffs.end(); ++itObjCoeff, ++itEqCoeff)
            *itEqCoeff = -*itObjCoeff;
        for (; itEqCoeff != _eqs.front().coeffs.end(); ++itEqCoeff)
            *itEqCoeff = 0;
    }

    int i = 0;
    auto itEq = _eqs.begin()+1;
    for (auto itConstr = constrs.begin(); itConstr != constrs.end(); ++itEq, ++itConstr, ++i) {
        _vars.at(i+1) = VarNb + i;
        itEq->result = itConstr->result;
        itEq->coeffs.resize(VarNb + EquNb);
        {auto itEqCoeff = itEq->coeffs.begin();
        for (auto itConstrCoeff = itConstr->coeffs.begin(); itConstrCoeff != itConstr->coeffs.end(); ++itConstrCoeff, ++itEqCoeff)
            *itEqCoeff = *itConstrCoeff;
        for (int j = 0; itEqCoeff != itEq->coeffs.end(); ++itEqCoeff, ++j)
            *itEqCoeff = j == i ? 1 : 0;
        }
    }
}

typename Simplex::Output Simplex::solve()
{
    const size_t EquNb = _eqs.size();
    Equation &objEq = _eqs.front();
    while (std::any_of(objEq.coeffs.begin(), objEq.coeffs.end(), [](Variable i) { return i < 0; })) {
        if (std::any_of(_eqs.begin()+1, _eqs.end(), [](const Equation &eq) {return eq.result < 0;}))
            throw std::range_error("No solution exist");

        size_t pivotCol = std::min_element(objEq.coeffs.begin(), objEq.coeffs.end()) - objEq.coeffs.begin();

        std::vector<double> ratios(EquNb, HUGE_VAL);
        for (size_t i = 1; i < EquNb; ++i)
            if (_eqs.at(i).coeffs.at(pivotCol) != 0 && _eqs.at(i).result * _eqs.at(i).coeffs.at(pivotCol) >= 0)
                ratios.at(i) = _eqs.at(i).result / _eqs.at(i).coeffs.at(pivotCol);
        auto pivotRow = _eqs.begin() + (std::min_element(ratios.begin(), ratios.end()) - ratios.begin());
        Variable pivotValue = pivotRow->coeffs.at(pivotCol);
        if (pivotValue == 0) throw std::runtime_error("Unexpected 0 pivot value");

        _vars.at(pivotRow - _eqs.begin()) = pivotCol;
        pivotRow->result /= pivotValue;
        for (Variable &coeff : pivotRow->coeffs)
            coeff /= pivotValue;

        for (auto it = _eqs.begin(); it != _eqs.end(); ++it) {
            Variable ratio = it->coeffs.at(pivotCol);
            if (ratio == 0 || it == pivotRow) continue;
            for (auto itC = it->coeffs.begin(), itRef = pivotRow->coeffs.begin(); itRef != pivotRow->coeffs.end(); ++itC, ++itRef)
                *itC -= ratio * *itRef;
            it->result -= ratio * pivotRow->result;
        }
    }
    if (std::any_of(_eqs.begin()+1, _eqs.end(), [](const Equation &eq) {return eq.result < 0;}))
        throw std::range_error("No solution exist");

    Results res(objEq.coeffs.size()-_eqs.size()+1, 0);
    for (size_t i = 0; i < objEq.coeffs.size()-_eqs.size()+1; ++i) {
        auto it = std::find(_vars.begin(), _vars.end(), i);
        if (it != _vars.end())
            res.at(i) = _eqs.at(it-_vars.begin()).result;
    }
    return {_eqs.at(0).result, res};
}

}