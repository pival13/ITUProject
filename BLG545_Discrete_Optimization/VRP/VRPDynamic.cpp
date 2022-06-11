#include "VRPDynamic.h"

#include <algorithm>
#include <numeric>

#define NEAREST_NODES 5ULL
#define STATES_SIZE   10'000ULL

VRPDynamic::~VRPDynamic() {}
VRPDynamic::VRPDynamic(size_t nbVeh, size_t cap, const Customers &custs)
    :   _nbVehicle(nbVeh), _capacity(cap), _customers(custs)
    ,   _distances(custs.size(), std::vector(custs.size(), 0.))
{
    for (size_t i = 0; i < custs.size(); ++i)
        for (size_t j = i+1; j < custs.size(); ++j)
            _distances[i][j] = _distances[j][i] = sqrt(
                pow(custs.at(j).pos.x-custs.at(i).pos.x, 2) +
                pow(custs.at(j).pos.y-custs.at(i).pos.y, 2)
            );
}

VRPDynamic::Solution VRPDynamic::run() const
{

    size_t housekeepIndex = _customers.front().index;
    std::list<size_t> GTR = solveTSP();

    Solution sol(_nbVehicle);
    auto it = ++GTR.cbegin();
    for (size_t i = 0; i < _nbVehicle; ++i) {
        auto itEnd = std::find(it, GTR.cend(), housekeepIndex);
        sol[i].address = std::list(it, ++itEnd);
        sol[i].address.push_front(housekeepIndex);
        sol[i].dist = 0.;
        for (auto it = sol[i].address.cbegin(); it != --sol[i].address.cend();)
            sol[i].dist += _distances.at(*(it++)).at(*it);
        sol[i].offer = _capacity - std::reduce(sol[i].address.cbegin(), sol[i].address.cend(), 0ULL, [this](size_t v, size_t index) {return v+_customers.at(index).demand;});
        it = itEnd;
    }
    return sol;
}

std::list<size_t> VRPDynamic::solveTSP() const
{
    using State = std::pair<double,std::list<size_t>>;
    size_t initNodeIdx = _customers.front().index;
    size_t totalDemand = std::reduce(_customers.cbegin(), _customers.cend(), 0ULL, [](size_t v, const Customer &c) {return v+c.demand;});

    std::list<size_t> GTR;
    for (size_t i = 1; i < _customers.size(); ++i)
        GTR.push_back(i);
    for (size_t i = 0; i < _nbVehicle; ++i)
        GTR.push_back(initNodeIdx);

    std::list<State> states = { {0., {initNodeIdx}} };
    for (size_t iter = 0; iter < GTR.size(); ++iter) {
        std::list<State> tmpStates;

        for (const auto &pair : states) {
            std::list<size_t> closestNeighbours = GTR;
            size_t lastNodeIdx = pair.second.back();
            auto lastBegin = std::find(closestNeighbours.crbegin(), closestNeighbours.crend(), initNodeIdx).base();
            size_t remainingDemand = std::reduce(pair.second.cbegin(), pair.second.cend(), totalDemand, [this](size_t v, size_t idx) {return v-_customers.at(idx).demand;});
            size_t remainingTours = _nbVehicle - std::count(++pair.second.cbegin(), pair.second.cend(), initNodeIdx);
            size_t remainingOffer = std::reduce(pair.second.crbegin(), std::find(pair.second.crbegin(), pair.second.crend(), initNodeIdx), _capacity, [this](size_t v, size_t idx) {return v-_customers.at(idx).demand;});

            std::list<size_t> alreadyVisited = pair.second;
            bool initNodePass = false;
            for (auto it = closestNeighbours.cbegin(), it2 = it; it != closestNeighbours.cend();) {
                // Already visited
                if ((it2 = std::find(++alreadyVisited.cbegin(), alreadyVisited.cend(), *it)) != alreadyVisited.cend()) {
                    alreadyVisited.erase(it2);
                // Duplicate end tour
                } else if (*it == initNodeIdx && initNodePass) {
                // End tour but not enough offer
                } else if (*it == initNodeIdx && remainingDemand > _capacity * (remainingTours-1)) {
                // Already present with others to visit
                } else if (*it == lastNodeIdx && pair.second.size()+remainingTours != GTR.size()) {
                // Too high demand
                } else if (_customers.at(*it).demand > remainingOffer) {
                // Valid node
                } else {
                    if (*it == initNodeIdx)
                        initNodePass = true;
                    ++it;
                    continue;
                }
                closestNeighbours.erase(it++);
            }

            /*std::list<size_t>::const_iterator it;
            // Erase already visited
            for (it = ++alreadyVisited.begin(); it != alreadyVisited.end(); ++it)
                closestNeighbours.erase(std::find(closestNeighbours.cbegin(), closestNeighbours.cend(), *it));
            // Erase break node when remaining impossible
            //if (std::reduce(closestNeighbours.cbegin(), closestNeighbours.cend(), 0ULL, [this](size_t v, size_t idx) {return v+_customers.at(idx).demand;}) > _capacity * (std::count(closestNeighbours.cbegin(), closestNeighbours.cend(), _customers.front().index)-1))
            if (remainingDemand > _capacity * (remainingTours-1))
                while ((it = std::find(closestNeighbours.cbegin(), closestNeighbours.cend(), initNodeIdx)) != closestNeighbours.cend())
                    closestNeighbours.erase(it);
            // Erase undoable node
            //while ((it = std::find_if(closestNeighbours.cbegin(), closestNeighbours.cend(), [this,&alreadyVisited](size_t index) {
            //    return _customers.at(index).demand > _capacity - std::reduce(alreadyVisited.crbegin(), std::find(alreadyVisited.crbegin(), alreadyVisited.crend(), _customers.front().index), 0ULL, [this](size_t v, size_t idx) {return v+_customers.at(idx).demand;});
            //})) != closestNeighbours.cend())
            while ((it = std::find_if(closestNeighbours.cbegin(), closestNeighbours.cend(), [this,remainingOffer](size_t index) { return _customers.at(index).demand > remainingOffer; })) != closestNeighbours.cend())
                closestNeighbours.erase(it);
            // Erase all current node
            while ((it = std::find(closestNeighbours.cbegin(), closestNeighbours.cend(), lastNodeIdx)) != closestNeighbours.cend())
                closestNeighbours.erase(it);*/

            // Restrict number of node
            if (closestNeighbours.size() > NEAREST_NODES) {
                closestNeighbours.sort([this,lastNodeIdx](size_t i1, size_t i2) { return _distances[lastNodeIdx][i1] < _distances[lastNodeIdx][i2]; });
                closestNeighbours.resize(NEAREST_NODES);
            }

            for (size_t nodeIdx : closestNeighbours) {
                std::list<size_t> state = pair.second;
                state.push_back(nodeIdx);
                tmpStates.push_back({pair.first+_distances[lastNodeIdx][nodeIdx], state});
            }
        }

        tmpStates.sort([](const State &s1, const State &s2) {
            if (s1.second.back() != s2.second.back())
                return s1.second.back() < s2.second.back();
            return s1.first < s2.first;
        });
        tmpStates.unique([this](const State &s1, const State &s2) {
            if (s1.second.back() != s2.second.back()) return false;
            for (size_t i = 0; i < _customers.size(); ++i)
                if (std::count(s1.second.cbegin(), s1.second.cend(), i) != std::count(s2.second.cbegin(), s2.second.cend(), i))
                    return false;
            return true;
        });
        tmpStates.sort([](const State &s1, const State &s2) { return s1.first < s2.first; });
        if (tmpStates.size() > STATES_SIZE)
            tmpStates.resize(STATES_SIZE);
        states = tmpStates;
    }
    return std::min_element(states.begin(), states.end(), [](const State &s1, const State &s2) {return s1.first < s2.first;})->second;
}