#pragma once

#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <utility>

class GCP_IG {
    public:
        using Node = size_t;
        using Edge = std::pair<Node,Node>;
        using Set = std::list<Node>;
        using Graph = std::unordered_map<Node, std::unordered_set<Node>>;

    public:
        GCP_IG(size_t nodeCount, const std::vector<Edge> &edges);
        ~GCP_IG();
        std::vector<size_t> run() const;

    private:
        std::list<Set> solveGreedy(const std::list<Node> &permutations) const;
        std::list<Set> solveReverse(const std::list<Set> &previousSets) const;
        std::list<Set> solveIncrease(const std::list<Set> &previousSets) const;
        std::list<Set> solveDecrease(const std::list<Set> &previousSets) const;
        std::list<Set> solveRandom(const std::list<Set> &previousSets, std::default_random_engine &rng) const;

    private:
        const Graph _graph;

};