#include "transport_router.h"

namespace NS_TransportCatalogue::TransportCatalogue_Router
{

GraphBuilder::RouterWeight GraphBuilder::RouterWeight::operator+(const RouterWeight& other) const
{
    return {weight + other.weight};
}

bool GraphBuilder::RouterWeight::operator<(const RouterWeight& other) const
{
    return weight < other.weight;
}

bool GraphBuilder::RouterWeight::operator>(const RouterWeight& other) const
{
    return !(*this < other);
}

GraphBuilder::GraphBuilder(const NS_TransportCatalogue::TransportCatalogue& catalog, RouterSettings settings)
    :catalog_(catalog), settings_(settings), graph_((catalog_.GetStopCount() * 2))
{
    Edgels_.reserve(catalog.GetStopCount() * 2);
    AddVertex(catalog_.GetStopListBegin(), catalog.GetStopListEnd());

    for (const auto& bus : catalog_.GetBusList())
    {
        CreateBusRoute(bus);
    }
}

GraphBuilder::GraphBuilder(const NS_TransportCatalogue::TransportCatalogue& catalog, InitStruct&& init_data)
    : catalog_(catalog), settings_(std::move(init_data.settings)), graph_(std::move(init_data.graph)), Edgels_(std::move(init_data.edgels)) {}

std::optional<unsigned int> GraphBuilder::GetBusID(std::string_view name) const
{
    auto out = catalog_.GetStopPtr(name);
    if (out == nullptr)
    {
        return std::nullopt;
    }
    return (out->id * 2); // в векторе с ребрами, информация о ребрах вершин (вход-выход) хранится не симметрично их добавлению, хранится со смещением в 2 
}

const graph::DirectedWeightedGraph<GraphBuilder::RouterWeight>& GraphBuilder::GetGraphRef()
{
    return graph_;
}

GraphBuilder::EdgeID GraphBuilder::GetEdge(size_t id) const
{
    return Edgels_[id];
}

GraphBuilder::Data GraphBuilder::GetData() const
{
    return {settings_, graph_, Edgels_};
}

} // end namespace NS_TransportCatalogue::TransportCatalogue_Router