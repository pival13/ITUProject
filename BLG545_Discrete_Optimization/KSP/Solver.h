#pragma once

#include <vector>
#include <tuple>

template<typename InType = double, typename OutType = InType>
class Solver {
    public:
        using Variable = InType;
        using Variables = std::vector<Variable>;
        using ObjectifValue = InType;
        using Result = OutType;
        using Results = std::vector<Result>;
        using Equation = struct { Variables coeffs; ObjectifValue result; };
        using Equations = std::vector<Equation>;
        using Output = std::tuple<ObjectifValue, Results>;

    public:
        virtual ~Solver() {};

        virtual Output solve() = 0;
};