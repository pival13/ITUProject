#include <fstream>
#include <iostream>
#include <string>
#include <regex>
#include <numeric>

#include "VRPDynamic.h"

#ifdef _DEBUG
#   define TRY
#   define CATCH
#else
#   define TRY try
#   define CATCH catch (const std::exception &e) { std::cerr << "Error: " << e.what() << std::endl; return 1; }
#endif

std::regex operator ""_r(const char *s, size_t ) { return std::regex(s); }

std::tuple<size_t, size_t, std::vector<VRPDynamic::Customer>> readProblem(std::istream &&stream)
{
    size_t capacity;
    size_t nbVehicule;
    std::vector<VRPDynamic::Customer> customers;

    std::smatch m;
    std::string line;
    std::getline(stream, line);

    if (!std::regex_match(line, m, R"((\d+)\s+(\d+)\s+(\d+))"_r))
        throw std::invalid_argument("Invalid first line");
    customers.resize(std::stoull(m[1]));
    nbVehicule = std::stoull(m[2]);
    capacity = std::stoull(m[3]);

    for (size_t i = 0; i < customers.size(); ++i) {
        std::getline(stream, line);
        if (!std::regex_match(line, m, R"((\d+)\s+(-?\d+(?:.\d+)?)\s+(-?\d+(?:.\d+)?))"_r))
            throw std::invalid_argument("Invalid line");
        if (std::stoull(m[1]) > capacity)
            throw std::invalid_argument("The demand is greater than the offer");
        customers[i].index = i;
        customers[i].demand = std::stoull(m[1]);
        customers[i].pos.x = std::stod(m[2]);
        customers[i].pos.y = std::stod(m[3]);
    }
    if (std::reduce(customers.cbegin(), customers.cend(), 0ULL, [](size_t v, const VRPDynamic::Customer &c){return v+c.demand;}) > capacity * nbVehicule)
        throw std::invalid_argument("The total demand is greater than the total offer");
    return {nbVehicule, capacity, customers};
}

int main(int argc, char **argv)
{
    if (argc != 2) return 1;

    TRY {
        auto [nbVehicule, capacity, customers] = readProblem(std::ifstream(argv[1]));
        auto tracks = VRPDynamic(nbVehicule, capacity, customers).run();

        std::cout << std::reduce(tracks.cbegin(), tracks.cend(), 0., [](double v, const VRPDynamic::Track &o) { return v + o.dist; }) << std::endl;
        for (const auto &track : tracks) {
            auto it = track.address.cbegin();
            std::cout << *(it++);
            if (it == track.address.cend())
                std::cout << " " << 0;
            else do
                std::cout << " " << *(it++);
                while (it != track.address.cend());
            std::cout << std::endl;
        }
    } CATCH;
    return 0;
}