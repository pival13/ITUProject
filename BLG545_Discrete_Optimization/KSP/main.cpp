#include <fstream>
#include <iostream>
#include <string>
#include <regex>

#include "BranchBounds.h"

using Variable = typename BIP::BranchAndBound::Variable;
using ObjectifValue = typename BIP::BranchAndBound::ObjectifValue;
using Equation = typename BIP::BranchAndBound::Equation;

std::pair<size_t, std::vector<std::pair<size_t, size_t>>> readProblem(std::istream &&is)
{
    size_t n, capacity;
    std::vector<std::pair<size_t, size_t>> items;
    std::string s;
    std::smatch m;
    
    std::getline(is, s);
    if (std::regex_match(s, m, std::regex(R"((\d+)\s+(\d+))"))) {
        n = std::stoull(m[1]);
        capacity = std::stoull(m[2]);
    } else
        throw std::invalid_argument("Invalid first line");
    for (size_t i = 0; i < n; ++i) {
        std::getline(is, s);
        if (std::regex_match(s, m, std::regex(R"((\d+)\s+(\d+))")))
            items.emplace_back(std::stoull(m[1]), std::stoull(m[2]));
        else
            throw std::invalid_argument("Invalid line");
    }

    return {capacity, items};
}

std::pair<Equation, Equation> generateFunctions(size_t capacity, const std::vector<std::pair<size_t, size_t>> &items)
{
    Equation objF;
        objF.result = ObjectifValue(0);
        objF.coeffs = std::vector<Variable>(items.size(), 0);
    Equation constrF;
        constrF.result = ObjectifValue(capacity);
        constrF.coeffs = std::vector<Variable>(items.size(), 0);

    size_t i = 0;
    for (const auto &[value, weight] : items) {
        objF.coeffs[i] = Variable(value);
        constrF.coeffs[i] = Variable(weight);
        ++i;
    }

    return {objF, constrF};
}
#include <chrono>
int main(int argc, char **argv)
{
    if (argc == 1) return 1;
    auto [capacity, items] = readProblem(std::ifstream(argv[1]));
    auto [objFunct, constrFunct] = generateFunctions(capacity, items);

    auto [res, values] = BIP::solveBranchBound(objFunct, {constrFunct});

    std::cout << (size_t)res << std::endl;
    for (auto it = values.begin(); it != values.end(); ++it)
        std::cout << (it != values.begin() ? " " : "") << (*it ? 1 : 0);
    std::cout << std::endl;
    return 0;
}