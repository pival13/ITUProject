#include "GCP_IG.h"

#include <numeric>
#include <algorithm>

static GCP_IG::Graph createGraph(size_t nodeCount, const std::vector<GCP_IG::Edge> &edges)
{
    GCP_IG::Graph graph;
    for (GCP_IG::Node n = 0; n < nodeCount; ++n) {
        std::unordered_set<GCP_IG::Node> neighbours;
        for (auto [node1,node2] : edges)
            if (node1 == node2)
                continue;
            else if (node1 == n)
                neighbours.insert(node2);
            else if (node2 == n)
                neighbours.insert(node1);
        graph.emplace(n, neighbours);
    }
    return graph;
}

GCP_IG::GCP_IG(size_t nodeCount, const std::vector<Edge> &edges)
    : _graph{createGraph(nodeCount, edges)} {}
GCP_IG::~GCP_IG() {}

std::vector<size_t> GCP_IG::run() const
{
    std::default_random_engine rng(20221208);

    std::list<Node> permutations(_graph.size()); std::iota(permutations.begin(), permutations.end(), 0);
    std::list<Set> sets = solveGreedy(permutations);
    for (size_t i = 0; i < 10000; ++i)
        if (i % 5 != 4)
            sets = solveDecrease(sets);
        else if (i % 25 != 24)
            sets = solveIncrease(sets);
        else
            sets = solveRandom(sets, rng);

    std::vector<size_t> colors(_graph.size());
    size_t c = 0;
    for (const Set &set : sets) {
        for (Node n : set)
            colors[n] = c;
        ++c;
    }
    return colors;
}

std::list<GCP_IG::Set> GCP_IG::solveGreedy(const std::list<Node> &permutations) const
{
    std::list<Set> sets;
    for (const Node &node : permutations) {
        const auto &neighbours = _graph.at(node);
        auto setIt = std::find_if(sets.begin(), sets.end(), [&neighbours](const Set &set) {
            for (const Node &n : set)
                if (neighbours.find(n) != neighbours.cend()) return false;
            return true;
        });
        if (setIt == sets.end())
            sets.push_back({node});
        else
            setIt->push_back(node);
    }
    return sets;
}

std::list<GCP_IG::Set> GCP_IG::solveReverse(const std::list<Set> &previousSets) const
{
    std::list<Node> permutations;
    for (auto setIt = previousSets.crbegin(); setIt != previousSets.crend(); ++setIt)
        permutations.insert(permutations.end(), setIt->cbegin(), setIt->cend());
    return solveGreedy(permutations);
}

std::list<GCP_IG::Set> GCP_IG::solveIncrease(const std::list<Set> &previousSets) const
{
    std::list<Set> sets = previousSets;
    sets.sort([](const Set &s1, const Set &s2) { return s1.size() > s2.size(); });
    sets.reverse();
    std::list<Node> permutations;
    for (Set &set : sets)
        permutations.splice(permutations.end(), set);
    return solveGreedy(permutations);
}

std::list<GCP_IG::Set> GCP_IG::solveDecrease(const std::list<Set> &previousSets) const
{
    std::list<Set> sets = previousSets;
    sets.sort([](const Set &s1, const Set &s2) { return s1.size() < s2.size(); });
    sets.reverse();
    std::list<Node> permutations;
    for (Set &set : sets)
        permutations.splice(permutations.end(), set);
    return solveGreedy(permutations);
}

std::list<GCP_IG::Set> GCP_IG::solveRandom(const std::list<Set> &previousSets, std::default_random_engine &rng) const
{
    std::vector<Set> sets(previousSets.cbegin(), previousSets.cend());
    std::shuffle(sets.begin(), sets.end(), std::default_random_engine(rng()));
    std::list<Node> permutations;
    for (Set &set : sets)
        permutations.splice(permutations.end(), set);
    return solveGreedy(permutations);
}