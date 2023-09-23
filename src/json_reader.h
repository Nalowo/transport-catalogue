#pragma once
#include <memory>
#include <map>
#include <filesystem>

#include "request_handler.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"


namespace NS_TransportCatalogue::Interfaces
{
class JsonReader final : public RequestHandler
{
private:

    using Path = std::filesystem::path;
    using DWGraph = graph::DirectedWeightedGraph<TransportCatalogue_Router::GraphBuilder::RouterWeight>;
    using Router = graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>;
    using Graph_ptr = std::unique_ptr<TransportCatalogue_Router::GraphBuilder>;
    using Router_ptr = std::unique_ptr<graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>>;

    std::unique_ptr<json::Document> json_data_{nullptr};
    std::unique_ptr<MapRenderer> render_{nullptr};
    std::optional<Path> file_path_{std::nullopt};
    MapRenderer::RenderSetting render_settings_;
    

    TransportCatalogue_Router::RouterSettings router_settings_;
    Graph_ptr graph_builder_{nullptr};
    Router_ptr router_{nullptr};

    void ReadContent();
    json::Node ProcessRequest();
    
    TransportCatalogue::BusInput ReadBus(const json::Dict& value) const;
    std::pair<domain::Stop, Stop_to_Stop_len> ReadStop(const json::Dict& value) const;
    MapRenderer::RenderSetting ReadRenderSetting(const json::Dict& value) const;
    json::Node ReturnStop(const json::Dict& value) const;
    json::Node ReturnBus(const json::Dict& value) const;
    json::Node ReturnRoute(const json::Dict& value);
    svg::Color GetColor(const json::Node& color_array) const;
    void ParseArrayStopAndBus(const json::Array& array);
    void ParseSerializationSettings(const json::Dict& value);

    void CreateRouter();


public:
    JsonReader(TransportCatalogue& catalog): RequestHandler(catalog) {}
    ~JsonReader() = default;

    const MapRenderer::RenderSetting& GetRenderSettings() const;
    const TransportCatalogue_Router::RouterSettings& GetRouterSettings() const;
    const TransportCatalogue_Router::GraphBuilder* GetGraphBuilderPtr() const;
    const graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>* GetRouterPtr() const;
    void ReadInput(std::istream& instream) override;
    void PrintRequest(std::ostream& outstream) override;
    void RenderMap(std::ostream& os);
    std::optional<JsonReader::Path> GetFilePath();
    bool RunCreateRouter();
    void SetMapReanderSettings(MapRenderer::RenderSetting&& in);
    void SetRouterSettings(TransportCatalogue_Router::RouterSettings&& settings);
    void InitRouter(TransportCatalogue_Router::GraphBuilder::InitStruct&& graph_builder_init, Router::InitStruct&& router_init);
};
} // end namespace NS_TransportCatalogue::Interfaces
