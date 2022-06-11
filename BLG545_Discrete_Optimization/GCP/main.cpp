#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

#include "GCP_IG.h"

std::pair<size_t, std::vector<std::pair<size_t, size_t>>> readProblem(std::istream &&is)
{
    size_t nodes;
    std::vector<std::pair<size_t, size_t>> edges;
    std::string s;
    std::smatch m;
    
    std::getline(is, s);
    if (std::regex_match(s, m, std::regex(R"((\d+)\s+(\d+))"))) {
        nodes = std::stoull(m[1]);
        edges.resize(std::stoull(m[2]));
    } else
        throw std::invalid_argument("Invalid first line");
    for (size_t i = 0; i < edges.size(); ++i) {
        std::getline(is, s);
        if (std::regex_match(s, m, std::regex(R"((\d+)\s+(\d+))")))
            edges.at(i) = {std::stoull(m[1]), std::stoull(m[2])};
        else
            throw std::invalid_argument("Invalid line");
    }

    return {nodes, edges};
}

int main(int argc, char **argv)
{
    if (argc == 1) return 1;
    auto [nodes, edges] = readProblem(std::ifstream(argv[1]));

    auto res = GCP_IG(nodes, edges).run();

    std::cout << *std::max_element(res.begin(), res.end()) + 1 << std::endl;
    for (auto it = res.begin(); it != res.end(); ++it)
        std::cout << (it != res.begin() ? " " : "") << *it;
    std::cout << std::endl;
    return 0;
}