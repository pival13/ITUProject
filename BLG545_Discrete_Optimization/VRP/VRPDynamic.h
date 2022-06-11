#pragma once

#include <vector>
#include <list>

class VRPDynamic {
    public:
        typedef struct { double x; double y; } Position;
        typedef struct { size_t index; size_t demand; Position pos; } Customer;
        typedef struct { size_t offer; std::list<size_t> address; double dist; } Vehicle;
        using Customers = std::vector<Customer>;
        using Vehicles = std::vector<Vehicle>;
        using Track = Vehicle;
        using Solution = Vehicles;

    public:
        VRPDynamic(size_t nbVehicle, size_t capacity, const Customers &customers);
        ~VRPDynamic();
    
    public:
        Solution run() const;

    private:
        std::list<size_t> solveTSP() const;

    private:
        const size_t _nbVehicle;
        const size_t _capacity;
        const Customers _customers;
        std::vector<std::vector<double>> _distances;
};