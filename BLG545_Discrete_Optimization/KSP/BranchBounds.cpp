#include "BranchBounds.h"

#include <queue>
#include <cmath>
#include <iostream>
#include <algorithm>

#define MAX_INSTANCES 10'000ULL

namespace BIP {

BranchAndBound::BranchAndBound(const Equation &obj, const Equations &constrs)
    :   _objEq(obj)
    ,   _constEqs(constrs)
    ,   optimalValues(obj.coeffs.size())
{}

typename BranchAndBound::Output BranchAndBound::solve()
{
    std::list<std::tuple<ObjectifValue, State>> states = { {HUGE_VAL, State(_objEq.coeffs.size(), Undefined)} };

    int a = 0;
    uint16_t counter = 0;
    while (!states.empty()) {
        const auto &[_, state] = states.front();
        const auto [objF, constrFs] = generateFunctions(state);
        ObjectifValue objInt = objF.result;

        LP::Simplex::ObjectifValue obj = 0.;
        LP::Simplex::Results result;
        try {
            std::tie(obj, result) = LP::solveSimplex(objF, constrFs);
            if (std::floor(obj) <= optimalRes)
                goto nextState; // Break 1: Optimal res < Saved res
        } catch (const std::range_error &) {
            goto nextState; // Break 2: No solution
        }

        for (size_t i = 0; i < objF.coeffs.size(); ++i)
            if (result[i] >= 1.)
                objInt += objF.coeffs[i];
        if (objInt > optimalRes) {
            optimalRes = objInt;
            for (size_t i = 0, iVal = 0; i < state.size(); ++i)
                if (state[i] == Undefined)
                    optimalValues[i] = result[iVal++] >= 1.;
                else
                    optimalValues[i] = state[i] == True;
            states.remove_if([objInt](const std::tuple<ObjectifValue, State> &state) { return std::get<0>(state) <= objInt; });
        }
        if (objInt == obj)
            goto nextState; // Break 3: Solution is binarized

        try {
            obj = std::floor(obj);
            std::list<State> newStates = getNewState(state, result);
            std::transform(newStates.begin(), newStates.end(), states.insert(states.end(), newStates.size(), {}),
                [obj](const State &newState) { return std::tuple<ObjectifValue,State>(obj, newState); });
        } catch (const std::range_error &) {
            std::cerr << "Potential non-optimal solution" << std::endl;
        }
        
        if (states.size() > MAX_INSTANCES) {
            states.sort([](const auto &v1, const auto &v2) { return std::get<0>(v1) > std::get<0>(v2); });
            for (size_t count = states.size(); count > MAX_INSTANCES; --count)
                states.pop_back();
        }

nextState:
        states.pop_front();
    }
    if (optimalRes == -HUGE_VAL)
        throw std::range_error("No solution exist");
    return {optimalRes, optimalValues};
}

std::tuple<LP::Simplex::Equation, LP::Simplex::Equations> BranchAndBound::generateFunctions(const State &state) const
{
    const size_t EquNb = _constEqs.size();
    const size_t VarNb = std::count(state.begin(), state.end(), Undefined);

    LP::Simplex::Equation obj;
        obj.result = _objEq.result;
        obj.coeffs = std::vector(VarNb, 0.);
    LP::Simplex::Equation tmp;
        tmp.result = 1;
        tmp.coeffs = std::vector(VarNb, 0.);
    LP::Simplex::Equations constrs = std::vector(EquNb + VarNb, tmp);

    for (size_t i = 0; i < EquNb; ++i)
        constrs.at(i).result = _constEqs.at(i).result;
    for (size_t iOrig = 0, iNew = 0; iOrig < state.size(); ++iOrig) {
        if (state.at(iOrig) == Undefined) {
            obj.coeffs.at(iNew) = _objEq.coeffs.at(iOrig);
            for (size_t i = 0; i < EquNb; ++i)
                constrs.at(i).coeffs.at(iNew) = _constEqs.at(i).coeffs.at(iOrig);
            constrs.at(EquNb + iNew).coeffs.at(iNew) = 1;
            ++iNew;
        } else if (state.at(iOrig) == True) {
            obj.result += _objEq.coeffs.at(iOrig);
            for (int i = 0; i < EquNb; ++i)
                constrs.at(i).result -= _constEqs.at(i).coeffs.at(iOrig);
        }
    }
    return {obj, constrs};
}

std::list<BranchAndBound::State> BranchAndBound::getNewState(const State &state, const LP::Simplex::Results &result) const
{
    std::list<State> states;
    size_t stateSize = state.size();

    std::vector<Value> values(stateSize, Undefined);
    size_t possibilities = 1;
    for (size_t i = 0, j = 0; i < stateSize; ++i) {
        if (state[i] == Undefined) {
            if (result[j++] >= 1.) {
                values[i] = True;
                possibilities = possibilities << 1;
            } else if (result[j-1] > 0)
                values[i] = False;
        }
    }
    if (possibilities == 0)
        throw std::range_error("Failed to evaluate every possibilities");

    for (size_t i = 0; i < possibilities; ++i) {
        State newState = state;
        auto pos = newState.end();
        for (size_t j = 0, b = 0; j < stateSize; ++j)
            if (values[j] == True)
                newState[j] = (i & (1ULL << b++)) != 0 ? False : True;
            else if (i == 0 && values[j] == False)
                pos = newState.begin() + j;
        if (pos != newState.end())
            *pos = False;

        for (const Equation &eq : _constEqs) {
            ObjectifValue eval = eq.result;
            for (size_t i = 0; i < newState.size(); ++i)
                if (newState[i] == True)
                    eval -= eq.coeffs[i];
            if (eval < 0)
                goto ignoreState;
            bool ignore = true;
            for (size_t i = 0; i < newState.size(); ++i)
                if (newState[i] == Undefined) {
                    if (eval - eq.coeffs[i] < 0)
                        newState[i] = False;
                    else
                        ignore = false;
                }
            if (ignore)
                goto ignoreState;
        }

        states.push_back(newState);
ignoreState:
        continue;
    }
    return states;
}

}