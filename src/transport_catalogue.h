#pragma once

#include <algorithm>
#include <cstdlib>
#include <vector>
#include <list>
#include <unordered_map>
#include <tuple>
#include <optional>
#include <exception>
#include <stdexcept>
#include <set>
#include <numeric>
#include <memory>

#include "domain.h"

namespace NS_TransportCatalogue
{
using namespace domain;

class TransportCatalogue
{
    friend class DB_Worker;
public:

    struct BusInput
    {
        std::string name;
        std::vector<std::string_view> stops;
        domain::BussRootType type;
    }; // struct BusInput

    struct BusOutput
    {
        std::string_view name;
        std::vector<std::string_view> stops_vec;
        unsigned int route_length;
        double curvature;
        domain::BussRootType route_type;
    }; // struct BusOutput

    struct StopOutput
    {
        std::string_view name;
        std::vector<std::string_view> buses_throw_stop;
    }; //struct StopInput

private:

    std::list<Stop> stops_base_;
    std::list<Bus> bus_base_;
    std::unordered_map<std::string_view, Stop*> stops_index_table_;
    std::unordered_map<std::string_view, Bus*> bus_index_table_;
    std::unordered_map<std::pair<const Stop*, const Stop*>, unsigned int, StopPairHasher> length_stop_to_neighbor_;
    std::unordered_map<const Stop*, std::set<std::string_view>> bus_throw_stop_;


public:

    TransportCatalogue() = default;

    void AddStop(Stop&& input);
    void AddStop(std::list<Stop>&& input_stops, std::list<std::pair<std::string_view, std::vector<std::pair<std::string_view, unsigned int>>>>&& root_length);

    std::optional<TransportCatalogue::StopOutput> GetStop(std::string_view name) const noexcept;
    const Stop* GetStopPtr(std::string_view name) const;

    void AddBus(BusInput&& input);

    std::optional<TransportCatalogue::BusOutput> GetBus(std::string_view name) const;

    unsigned int CheckRouteLength(const Stop* stop, const Stop* stop_next) const;

    std::vector<const domain::Bus*> GetBusVector() const;
    size_t GetBusCount() const;
    const std::list<Bus>& GetBusList() const;
    std::list<Bus>::const_iterator GetBusListBegin() const;
    std::list<Bus>::const_iterator GetBusListEnd() const;
    
    size_t GetStopCount() const;
    const std::list<Stop>& GetStopList() const;
    std::list<Stop>::const_iterator GetStopListBegin() const;
    std::list<Stop>::const_iterator GetStopListEnd() const;
}; // class TransportCatalogue

class DB_Worker
{
public:

    virtual ~DB_Worker(){};

protected:
    // DB_Worker(const TransportCatalogue& catalog): catalog_(const_cast<TransportCatalogue&>(catalog)) {}
    DB_Worker(const TransportCatalogue& catalog): catalog_(const_cast<TransportCatalogue&>(catalog)) {}

    struct Deserealiz_TC_Fields
    {
        std::list<Stop>& stops_base_;
        std::list<Bus>& bus_base_;
        std::unordered_map<std::string_view, Stop*>& stops_index_table_;
        std::unordered_map<std::string_view, Bus*>& bus_index_table_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, unsigned int, StopPairHasher>& length_stop_to_neighbor_;
        std::unordered_map<const Stop*, std::set<std::string_view>>& bus_throw_stop_;
    };

    struct Serealiz_TC_Fields
    {
        const std::list<Stop>& stops_base_;
        const std::list<Bus>& bus_base_;
        const std::unordered_map<std::pair<const Stop*, const Stop*>, unsigned int, StopPairHasher>& length_stop_to_neighbor_;
    };

    Deserealiz_TC_Fields GetDeserealizFields()
    {
        return {
                    catalog_.stops_base_, 
                    catalog_.bus_base_,
                    catalog_.stops_index_table_, 
                    catalog_.bus_index_table_,
                    catalog_.length_stop_to_neighbor_, 
                    catalog_.bus_throw_stop_
                };
    }

    const Serealiz_TC_Fields GetSerealizFields() const
    {
        return {catalog_.stops_base_, catalog_.bus_base_, catalog_.length_stop_to_neighbor_};
    }

private:
    TransportCatalogue& catalog_;
}; // end class DB_Worker

} // namespace NS_TransportCatalogue


