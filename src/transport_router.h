#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <string>
#include <iostream>

namespace NS_TransportCatalogue::TransportCatalogue_Router
{

struct RouterSettings
{
    double bus_speed = 0;
    double bus_wait_time = 0;
};

class GraphBuilder
{
    template <typename Iter> class WeightCounter;
    template <typename> friend class WeightCounter;
public:

    struct EdgeID
    {
        std::string name;
        int span_count = 0;
        double weight = 0;
    };

    struct RouterWeight
    {
        double weight = 0;

        RouterWeight operator+(const RouterWeight& other) const;

        bool operator<(const RouterWeight& other) const;

        bool operator>(const RouterWeight& other) const;
    };

    struct InitStruct
    {
        RouterSettings settings;
        graph::DirectedWeightedGraph<RouterWeight> graph;
        std::vector<EdgeID> edgels;
    };

    struct Data
    {
        RouterSettings settings;
        const graph::DirectedWeightedGraph<RouterWeight>& graph;
        const std::vector<EdgeID>& edgels;
    };

    GraphBuilder(const NS_TransportCatalogue::TransportCatalogue& catalog, RouterSettings settings);
    GraphBuilder(const NS_TransportCatalogue::TransportCatalogue& catalog, InitStruct&& init_data);
    const graph::DirectedWeightedGraph<RouterWeight>& GetGraphRef();
    std::optional<unsigned int> GetBusID(std::string_view name) const;
    EdgeID GetEdge(size_t id) const;
    Data GetData() const;
private:

    const NS_TransportCatalogue::TransportCatalogue& catalog_;
    RouterSettings settings_;
    graph::DirectedWeightedGraph<RouterWeight> graph_;
    std::vector<EdgeID> Edgels_;

    template <typename Iter>
    class WeightCounter
    {
    private:
        const GraphBuilder& graph_;
    public:
        WeightCounter(const GraphBuilder& graph, Iter begin, Iter end):graph_(graph), cur_vertex(begin), end_vertex(end), next_vertex(begin) {}

        double GetWeight()
        {
            ++next_vertex;
            if (next_vertex == end_vertex)
            {
                return 0;
            }
            cur_weight += (graph_.catalog_.CheckRouteLength(*cur_vertex, *next_vertex)) / ((graph_.settings_.bus_speed * 1000) / 60);
            ++cur_vertex;

            return cur_weight;
        }

        double GetWeightReverse()
        {
            if (cur_vertex == end_vertex)
            {
                return 0;
            }
            --next_vertex;
            cur_weight += (graph_.catalog_.CheckRouteLength(*cur_vertex, *next_vertex)) / ((graph_.settings_.bus_speed * 1000) / 60);
            --cur_vertex;

            return cur_weight;
        }

        Iter cur_vertex;
        Iter end_vertex;
        double cur_weight = 0;
        Iter next_vertex;
    };

    template <typename Iter>
    void AddVertex(Iter begin, Iter end)
    {
        unsigned int i = 0;
        for (auto iter = begin; iter != end; ++iter)
        {
            Edgels_.push_back({(*iter).name, 0, settings_.bus_wait_time});
            graph_.AddEdge({i, i + 1, settings_.bus_wait_time});
            i += 2;
        }
    }

    template <typename Iter>
    void CreateRoundRoute(Iter iter_begin, Iter iter_end, std::string bus_name)
    {
        bool trig = true;
        for (auto stop = iter_begin; stop < iter_end; ++stop)
        {
            auto start_id = ((*stop)->id * 2) + 1;
            
            int span_count = 1;

            auto end_iter = trig ? iter_end - 1 : iter_end;
            WeightCounter weight{*this, stop, end_iter};

            for (auto to_stop = stop + 1; to_stop < end_iter; ++to_stop)
            {
                auto to_stop_id = ((*to_stop)->id * 2);
                graph_.AddEdge({start_id, to_stop_id, weight.GetWeight()});
                Edgels_.push_back({bus_name, span_count++, weight.cur_weight});
            }

            trig = false;
        }
    }

    template <typename Iter>
    void CreateRoundtripRoute(Iter iter_begin, Iter iter_end, std::string bus_name)
    {
        for (auto stop_to_right = iter_begin, stop_to_left = iter_end - 1; stop_to_right < iter_end; ++stop_to_right, --stop_to_left)
        {
            auto r_stop_id = ((*stop_to_right)->id * 2) + 1;
            auto l_stop_id = ((*stop_to_left)->id * 2) + 1;
            
            int span_count = 1;
            WeightCounter r_weight{*this, stop_to_right, iter_end};
            WeightCounter l_weight{*this, stop_to_left, iter_begin};

            for (auto to_r_stop = stop_to_right + 1, to_l_stop = stop_to_left - 1; to_r_stop < iter_end; ++to_r_stop, --to_l_stop)
            {
                auto to_r_stop_id = ((*to_r_stop)->id * 2);
                auto to_l_stop_id = ((*to_l_stop)->id * 2);
                graph_.AddEdge({r_stop_id, to_r_stop_id, r_weight.GetWeight()});
                graph_.AddEdge({l_stop_id, to_l_stop_id, l_weight.GetWeightReverse()});
                Edgels_.push_back({bus_name, span_count, r_weight.cur_weight});
                Edgels_.push_back({bus_name, span_count, l_weight.cur_weight});
                ++span_count;
            }
        }
    }

    void CreateBusRoute(const domain::Bus& bus)
    {
        if (bus.root_type == domain::BussRootType::CYCLE)
        {
            CreateRoundRoute(bus.stops.begin(), bus.stops.end(), bus.name);
        }
        else
        {
            CreateRoundtripRoute(bus.stops.begin(), bus.stops.end(), bus.name);
        }
    }
}; // end GraphBuilder

} // end namespace domain