#include "transport_catalogue.h"

namespace NS_TransportCatalogue
{
    using namespace domain;
void TransportCatalogue::AddStop(Stop&& input) 
{
    auto iter = stops_base_.insert(stops_base_.end(), std::move(input));
    iter->id = (stops_base_.size() - 1);
    stops_index_table_[iter->name] = &(*iter);
    bus_throw_stop_[&(*iter)];
} // AddStop


void TransportCatalogue::AddStop(std::list<Stop>&& input_stops, std::list<std::pair<std::string_view, std::vector<std::pair<std::string_view, unsigned int>>>>&& root_length) {
    for (auto& stop : input_stops) {
        AddStop(std::move(stop));
    }
    
    for (auto& stop_to_stop : root_length) {
        auto iter = stops_index_table_.find(stop_to_stop.first);
        for (auto& to_stop : stop_to_stop.second) {
            auto iter2 = stops_index_table_.find(to_stop.first);
            length_stop_to_neighbor_.insert({{iter->second, iter2->second}, to_stop.second});
        }
    }
} // AddStop container input

std::optional<TransportCatalogue::StopOutput> TransportCatalogue::GetStop(std::string_view name) const noexcept 
{
    const auto iter = stops_index_table_.find(name);
    
    if (iter == stops_index_table_.end())
    {
        return std::nullopt;
    }
    
    const auto iter_buses = bus_throw_stop_.find(iter->second);
    std::vector<std::string_view> out;
    if (!(iter_buses == bus_throw_stop_.end()))
    {
        out.reserve(iter_buses->second.size());
        
        if (iter == stops_index_table_.end()) 
        {
            return std::nullopt;
        }

        for (std::string_view i : iter_buses->second)
        {
            out.push_back(i);
        }
    }

    return TransportCatalogue::StopOutput{iter->second->name, std::move(out)};
} // GetStop

void TransportCatalogue::AddBus(BusInput&& input) 
{
    size_t index_val = 0;
    double distance = 0;
    auto iter_bus = bus_base_.insert(bus_base_.end(), {std::move(input.name), std::vector<const Stop*>{}, distance, input.type});
    iter_bus->id = (bus_base_.size() - 1);
    iter_bus->stops.reserve(input.stops.size());
    
    for  (auto stop : input.stops) {
        auto iter = stops_index_table_.find(stop);
        
        if (iter == stops_index_table_.end()) 
        {
            bus_base_.erase(iter_bus);
            throw std::invalid_argument("Stop - " + std::string{stop} + " - not found");
        }
        
        iter_bus->stops.push_back(iter->second);
        iter_bus->distance += index_val > 0 ? geo::ComputeDistance(iter_bus->stops[index_val - 1]->coordinates, iter_bus->stops[index_val]->coordinates) : 0;
        ++index_val;
        bus_index_table_.insert({iter_bus->name, &(*iter_bus)});
        bus_throw_stop_[iter->second].insert(iter_bus->name);
    }
} // AddBus

unsigned int TransportCatalogue::CheckRouteLength(const Stop* stop, const Stop* stop_next) const
{
    auto stop_to_nei = length_stop_to_neighbor_.find({stop, stop_next});
    if (stop_to_nei != length_stop_to_neighbor_.end())
    {
        return stop_to_nei->second;
    }
    else
    {
        stop_to_nei = length_stop_to_neighbor_.find({stop_next, stop});
        if (stop_to_nei != length_stop_to_neighbor_.end())
        {
            return stop_to_nei->second;
        }
    }
    return 0;
} // CheckRouteLength

std::optional<TransportCatalogue::BusOutput> TransportCatalogue::GetBus(std::string_view name) const 
{
    const auto iter = bus_index_table_.find(name);
    if (iter == bus_index_table_.end()) 
        return std::nullopt;
    std::vector<std::string_view> stops_vec;
    std::vector<unsigned int> lengt_vec;
    stops_vec.reserve(iter->second->stops.size());
    lengt_vec.reserve(iter->second->stops.size() * 2);

    if (iter->second->root_type == BussRootType::CYCLE)
    {
        for (auto i = iter->second->stops.begin(); i < iter->second->stops.end(); ++i)
        {
            stops_vec.push_back((*i)->name);
            if (std::next(i) != iter->second->stops.end()) 
            {
                lengt_vec.push_back(CheckRouteLength(*i, *std::next(i)));
            }
        }
    }
    else if (iter->second->root_type == BussRootType::FORWARD)
    {
        auto fi = iter->second->stops.begin();
        auto ri = iter->second->stops.rbegin();
        for (; fi < iter->second->stops.end() && ri < iter->second->stops.rend(); ++fi, ++ri)
        {
            stops_vec.push_back((*fi)->name);
            if (std::next(fi) != iter->second->stops.end() && std::next(ri) != iter->second->stops.rend())
            {
                lengt_vec.push_back(CheckRouteLength(*fi, *std::next(fi)));
                lengt_vec.push_back(CheckRouteLength(*ri, *std::next(ri)));
            }
            else
            {
                lengt_vec.push_back(CheckRouteLength(*fi, *fi));
            }
        }
    }
    unsigned int length = 0;
    length += std::accumulate(lengt_vec.begin(), lengt_vec.end(), 0);
    double curvature = 0;
    if (iter->second->root_type == BussRootType::FORWARD)
    {
        curvature = (length / 2) / iter->second->distance;
    }
    else
    {
        curvature = length / iter->second->distance;
    }
    return TransportCatalogue::BusOutput{iter->second->name, std::move(stops_vec), length, curvature, iter->second->root_type};
} // GetBus

std::vector<const domain::Bus*> TransportCatalogue::GetBusVector() const
{
    std::vector<const domain::Bus*> output;
    if (!bus_base_.empty())
    {
        output.reserve(bus_base_.size());
        for (const domain::Bus& bus : bus_base_)
        {
            output.push_back(&bus);
        }
    }
    return output;
} // GetBusVector

size_t TransportCatalogue::GetBusCount() const
{
    return bus_base_.size();
}
const std::list<Bus>& TransportCatalogue::GetBusList() const
{
    return bus_base_;
}
std::list<Bus>::const_iterator TransportCatalogue::GetBusListBegin() const
{
    return bus_base_.begin();
}
std::list<Bus>::const_iterator TransportCatalogue::GetBusListEnd() const
{
    return bus_base_.end();
}

size_t TransportCatalogue::GetStopCount() const
{
    return stops_base_.size();
} // GetStopCount

const std::list<Stop>& TransportCatalogue::GetStopList() const
{
    return stops_base_;
} // GetStopList

std::list<Stop>::const_iterator TransportCatalogue::GetStopListBegin() const
{
    return stops_base_.begin();
} // GetStopListBegin

std::list<Stop>::const_iterator TransportCatalogue::GetStopListEnd() const
{
    return stops_base_.end();
} // GetStopListEnd

const Stop* TransportCatalogue::GetStopPtr(std::string_view name) const
{
    auto iter = stops_index_table_.find(name);
    if (iter != stops_index_table_.end())
    {
        return iter->second;
    }

    return nullptr;
} // GetStopPtr

} // namespace NS_TransportCatalogue