#pragma once

#include <string>
#include <vector>

#include "geo.h"

namespace domain
{

enum class BussRootType {CYCLE, FORWARD};

struct Stop
{
    Stop(std::string name_in, geo::Coordinates coordinates_in): name(std::move(name_in)), coordinates(coordinates_in) {}
    
    std::string name;
    geo::Coordinates coordinates;
    size_t id = 0;
}; // struct Stop

struct Bus
{
    std::string name;
    std::vector<const Stop*> stops;
    double distance = 0;
    BussRootType root_type = BussRootType::FORWARD;
    size_t id = 0;
}; // struct Bus

struct StopPairHasher
{
    size_t operator() (const std::pair<const Stop*, const Stop*>& input_pair) const noexcept
    {
        return std::hash<const void*>{}(input_pair.first) + std::hash<const void*>{}(input_pair.second);
    }
}; // struct StopPairHasher
} // end namespace domain

