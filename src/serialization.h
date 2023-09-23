#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"

#include <iostream>
#include <filesystem>
#include <fstream>

#include "transport_catalogue.pb.h"


namespace NS_TransportCatalogue::Serealization_Worker
{

using Path = std::filesystem::path;

class Serealization final : public NS_TransportCatalogue::DB_Worker
{
public:
    using Path = std::filesystem::path;

    Serealization(const TransportCatalogue& catalog);
    
    void RunSerealization(std::ostream& output);

    void SetMapSettings(const Interfaces::MapRenderer::RenderSetting* settings);
    void SetRouterSettings(const TransportCatalogue_Router::RouterSettings* settings);
    void SetGraphBuilder(const TransportCatalogue_Router::GraphBuilder* builder);
    void SetRouter(const graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>* router);

private:

    using DWGraph = graph::DirectedWeightedGraph<TransportCatalogue_Router::GraphBuilder::RouterWeight>;

    DB_Worker::Serealiz_TC_Fields fields_;
    const TransportCatalogue_Router::RouterSettings* router_settings_ = nullptr;
    const Interfaces::MapRenderer::RenderSetting* map_settings_ = nullptr;
    const TransportCatalogue_Router::GraphBuilder* graph_builder_ptr_ = nullptr;
    const graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>* router_ptr_ = nullptr;

    transport_catalogue_serialize::TransportCatalogue CreateProtoTransportCatalogue() const;
    transport_catalogue_serialize::Stop CreateProtoStop(const domain::Stop& stop) const;
    transport_catalogue_serialize::Bus CreateProtoBus(const domain::Bus& bus) const;
    transport_catalogue_serialize::StopToStop CreateProtoStopToStop(const domain::Stop* from, const domain::Stop* to, unsigned int lenght) const;

    transport_catalogue_serialize::RenderSettings CreateProtoRenderSettings() const;
    transport_catalogue_serialize::RouterSettings CreateProtoRouterSettings(TransportCatalogue_Router::RouterSettings settings) const;
    transport_catalogue_serialize::DirectedWeightedGraph CreateProtoDWGraph(const DWGraph& graph) const;
    transport_catalogue_serialize::GraphBuilder CreateProtoGraphBuilder() const;
    transport_catalogue_serialize::Router CreateProtoRouter() const;
    transport_catalogue_serialize::Color CreateProtoColor(const svg::Color& color_in) const;
}; // end class Serealization

class Deserealization final : public NS_TransportCatalogue::DB_Worker
{
public:

    Deserealization(TransportCatalogue& catalog);

    void RunDeserealization(std::istream& input);
    void RunDeserealization(std::istream& input, Interfaces::JsonReader& reader);

private:

    DB_Worker::Deserealiz_TC_Fields fields_;
    transport_catalogue_serialize::TransportCatalogue desed_catalog_;

    void DeserealizationTC(std::istream& input);

    Interfaces::MapRenderer::RenderSetting CreateMapRenderSettings(transport_catalogue_serialize::RenderSettings& settings);
    graph::DirectedWeightedGraph<TransportCatalogue_Router::GraphBuilder::RouterWeight>::InitStruct CreateDWGraphInit(transport_catalogue_serialize::DirectedWeightedGraph* proto_graph);
    TransportCatalogue_Router::RouterSettings CreateRouterSettings(transport_catalogue_serialize::RouterSettings* settings);
    TransportCatalogue_Router::GraphBuilder::InitStruct CreateGraphBuilderInit(TransportCatalogue_Router::RouterSettings&& settings, transport_catalogue_serialize::GraphBuilder* proto_puilder);
    graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>::InitStruct CreateRouterInit(transport_catalogue_serialize::Router* proto_router);
    void FeedTCFieds();
    std::vector<domain::Stop*> FeedStops(google::protobuf::RepeatedPtrField<transport_catalogue_serialize::Stop>* stops_arr);
    void FeedFields(google::protobuf::RepeatedPtrField<transport_catalogue_serialize::Bus>* bus_arr, google::protobuf::RepeatedPtrField<transport_catalogue_serialize::StopToStop>* length_arr, std::vector<domain::Stop*>&& stops_id_index);
    void FeedBus(transport_catalogue_serialize::Bus* bus_in, const std::vector<domain::Stop*>& stops_id_index);
    void FeedLength(transport_catalogue_serialize::StopToStop* proto_st_to_st, const std::vector<domain::Stop*>& stops_id_index);
    svg::Color CreateColor(transport_catalogue_serialize::Color& color);

}; // end class Serealization

} // end namespace Serealization_Worker