#include <vector>
#include <string>
#include <list>
#include <algorithm>
#include <sstream>

#include "json_reader.h"


namespace NS_TransportCatalogue::Interfaces
{
    void JsonReader::ReadInput(std::istream& instream)
    {
        json_data_ = std::make_unique<json::Document>(json::Load(instream));
        
        ReadContent();
    }

    TransportCatalogue::BusInput JsonReader::ReadBus(const json::Dict& value) const
    {
        std::vector<std::string_view> stops;
        stops.reserve(value.at("stops").AsArray().size());

        for (const auto& stop : value.at("stops").AsArray())
        {
            stops.emplace_back(stop.AsString());
        }

        domain::BussRootType root_type = value.at("is_roundtrip").AsBool() ? domain::BussRootType::CYCLE : domain::BussRootType::FORWARD;

        return TransportCatalogue::BusInput{value.at("name").AsString(), std::move(stops), root_type};
    }

    std::pair<domain::Stop, RequestHandler::Stop_to_Stop_len> JsonReader::ReadStop(const json::Dict& value) const
    {
        std::vector<std::pair<std::string_view, unsigned int>> stops_len;
        stops_len.reserve(value.at("road_distances").AsDict().size());

        for (const auto& [key, len] : value.at("road_distances").AsDict())
        {
            stops_len.emplace_back(key, len.AsInt());
        }

        return {{value.at("name").AsString(), std::move(geo::Coordinates{value.at("latitude").AsDouble(), value.at("longitude").AsDouble()})}, {value.at("name").AsString(), std::move(stops_len)}};
    }

    svg::Color JsonReader::GetColor(const json::Node& color_in) const
    {
        svg::Color output;

        if (color_in.IsString())
        {
            output = color_in.AsString();
        }
        else if (color_in.IsArray())
        {
            const auto& color_array = color_in.AsArray();
            if (color_array.size() > 3)
            {
                output = std::move(svg::Color{svg::Rgba{static_cast<uint8_t>(color_array[0].AsInt()), static_cast<uint8_t>(color_array[1].AsInt()), static_cast<uint8_t>(color_array[2].AsInt()), color_array[3].AsDouble()}});
            }
            else
            {
                output = std::move(svg::Color{svg::Rgb{static_cast<uint8_t>(color_array[0].AsInt()), static_cast<uint8_t>(color_array[1].AsInt()), static_cast<uint8_t>(color_array[2].AsInt())}});
            }
        }
        
        return output;
    }

    MapRenderer::RenderSetting JsonReader::ReadRenderSetting(const json::Dict& value) const
    {
        MapRenderer::RenderSetting settings;

        settings.width = value.at("width").AsDouble();
        settings.height = value.at("height").AsDouble();
        settings.padding = value.at("padding").AsDouble();
        settings.line_width = value.at("line_width").AsDouble();
        settings.stop_radius = value.at("stop_radius").AsDouble();
        settings.bus_label_font_size = value.at("bus_label_font_size").AsDouble();
        const auto& bus_l_offset = value.at("bus_label_offset").AsArray();
        settings.bus_label_offset = {bus_l_offset[0].AsDouble(), bus_l_offset[1].AsDouble()};
        settings.stop_label_font_size = value.at("stop_label_font_size").AsInt();
        const auto& stop_l_offset = value.at("stop_label_offset").AsArray();
        settings.stop_label_offset = {stop_l_offset[0].AsDouble(), stop_l_offset[1].AsDouble()};

        settings.underlayer_color = GetColor(value.at("underlayer_color"));

        settings.underlayer_width = value.at("underlayer_width").AsDouble();
        const auto& color_pal = value.at("color_palette").AsArray();
        settings.color_palette.reserve(color_pal.size());

        for (const auto& color : color_pal)
        {
            settings.color_palette.push_back(GetColor(color));
        }

        return settings;
    }

    void JsonReader::CreateRouter()
    {
        if (router_settings_.bus_speed == 0)
        {
            throw std::invalid_argument("router settings error: bus speed cannot be 0"); 
        }
        graph_builder_ = std::make_unique<TransportCatalogue_Router::GraphBuilder>(db_, router_settings_);
        router_ = std::make_unique<graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>>(graph_builder_->GetGraphRef());
    }

    const MapRenderer::RenderSetting& JsonReader::GetRenderSettings() const
    {
        return render_settings_;
    }

    json::Node JsonReader::ReturnStop(const json::Dict& value) const
    {
        json::Builder out;
        auto stop_answer = db_.GetStop(value.at("name").AsString());
        if (stop_answer)
        {
            json::Array buses_vec;
            buses_vec.reserve(stop_answer->buses_throw_stop.size());
            for (auto& bus : stop_answer->buses_throw_stop)
            {
                buses_vec.emplace_back(json::Node{std::string{bus}});
            }

            return out.StartDict().Key("buses"s).Value(std::move(buses_vec))
                                    .Key("request_id"s).Value(value.at("id").AsInt())
                                    .EndDict().Build();
        }
        else
        {
            return out.StartDict().Key("request_id"s).Value(value.at("id").AsInt()).Key("error_message"s).Value("not found"s).EndDict().Build();
        }
    }

    json::Node JsonReader::ReturnBus(const json::Dict& value) const
    {
        json::Builder out;
        auto bus_answer = db_.GetBus(value.at("name").AsString());
        if (bus_answer)
        {
            int count_stops_all = bus_answer->route_type == domain::BussRootType::FORWARD ? (static_cast<int>(bus_answer->stops_vec.size()) * 2 - 1) : static_cast<int>(bus_answer->stops_vec.size());
            
            std::sort(bus_answer->stops_vec.begin(), bus_answer->stops_vec.end());
            bus_answer->stops_vec.erase(std::unique(bus_answer->stops_vec.begin(), bus_answer->stops_vec.end()), bus_answer->stops_vec.end());
            
            int unique_stop_count = static_cast<int>(bus_answer->stops_vec.size());

            return out.StartDict()
                        .Key("curvature"s).Value(bus_answer->curvature)
                        .Key("request_id"s).Value(value.at("id").AsInt())
                        .Key("route_length"s).Value(static_cast<int>(bus_answer->route_length))
                        .Key("stop_count"s).Value(count_stops_all)
                        .Key("unique_stop_count"s).Value(unique_stop_count)
                        .EndDict().Build();
        }
        else
        {
            return out.StartDict().Key("request_id"s).Value(value.at("id").AsInt()).Key("error_message"s).Value("not found"s).EndDict().Build();
        }
    }

    json::Node JsonReader::ReturnRoute(const json::Dict& value)
    {
        if (graph_builder_ == nullptr || router_ == nullptr)
        {
            CreateRouter();
        }
        json::Builder out;
        std::optional<unsigned int> stop_from = graph_builder_->GetBusID(value.at("from"s).AsString());
        std::optional<unsigned int> stop_to = graph_builder_->GetBusID(value.at("to"s).AsString());

        if (stop_from != std::nullopt && stop_to != std::nullopt)
        {
            auto route = router_->BuildRoute(*stop_from, *stop_to);
            if (route != std::nullopt)
            {
                json::Array items;
                for (size_t item : route->edges)
                {
                    json::Dict item_node;
                    TransportCatalogue_Router::GraphBuilder::EdgeID edge = graph_builder_->GetEdge(item);

                    item_node.insert({"time"s, edge.weight});

                    if (edge.span_count == 0)
                    {
                        item_node.insert({"stop_name"s, std::string{edge.name}});
                        item_node.insert({"type"s, "Wait"s});
                    }
                    else
                    {
                        item_node.insert({"bus"s, std::string{edge.name}});
                        item_node.insert({"type"s, "Bus"s});
                        item_node.insert({"span_count"s, edge.span_count});
                    }

                    items.push_back(std::move(item_node));
                }

                return out.StartDict().Key("items"s).Value(std::move(items))
                                    .Key("request_id"s).Value(value.at("id"s))
                                    .Key("total_time"s).Value(route->weight.weight)
                                    .EndDict().Build();
            }
        }

        return out.StartDict().Key("request_id"s).Value(value.at("id"s).AsInt()).Key("error_message"s).Value("not found"s).EndDict().Build();
    }

    bool JsonReader::RunCreateRouter()
    {
        if (graph_builder_ == nullptr || router_ == nullptr)
        {
            CreateRouter();
            return true;
        }
        return false;
    }

    void JsonReader::ParseArrayStopAndBus(const json::Array& array)
    {
        std::list<TransportCatalogue::BusInput> bus_to_add;
        std::list<Stop_to_Stop_len> root_length;
        std::list<domain::Stop> stops_to_add;
    
        for (const auto& value : array)
        {
            const json::Dict& current_value = value.AsDict();

            const auto& curr_type = current_value.at("type").AsString();
            if (curr_type == "Bus")
            {
                bus_to_add.push_back(ReadBus(current_value));
            }
            else if (curr_type == "Stop")
            {

                auto stop = ReadStop(current_value);

                stops_to_add.push_back(std::move(stop.first));
                root_length.push_back(stop.second);

            }
        }
        RequestHandler::UploadContent(std::move(stops_to_add), std::move(root_length), std::move(bus_to_add));
    }

    TransportCatalogue_Router::RouterSettings ParseRoutingSettings(const json::Dict& value)
    {
        return {value.at("bus_velocity").AsDouble(), value.at("bus_wait_time").AsDouble()};
    }

    void JsonReader::ReadContent()
    {
        const auto& json_root = json_data_->GetRoot().AsDict();
        {
            const auto iter_content = json_root.find("base_requests");
            if (iter_content != json_root.end())
            {
                ParseArrayStopAndBus(iter_content->second.AsArray());
            }
        }
        {
            const auto iter_content = json_root.find("routing_settings");
            if (iter_content != json_root.end())
            {
                router_settings_ = ParseRoutingSettings(iter_content->second.AsDict());
            }
        }
        {
            const auto iter_content = json_root.find("render_settings");
            if (iter_content != json_root.end())
            {
                render_settings_ = ReadRenderSetting(iter_content->second.AsDict());
            }
        }
        {
            const auto iter_content = json_root.find("serialization_settings");
            if (iter_content != json_root.end())
            {
                ParseSerializationSettings(iter_content->second.AsDict());
            }
        }
    }

    void JsonReader::PrintRequest(std::ostream& outstream)
    {
        json::Print(json::Document{ProcessRequest()}, outstream);
    }

    json::Node JsonReader::ProcessRequest()
    {
        using namespace std::string_literals;

        const json::Array& content = json_data_->GetRoot().AsDict().at("stat_requests").AsArray();

        json::Array output;
        output.reserve(content.size()); 
        for (const auto& request : content)
        {
            const json::Dict& request_data = request.AsDict();
            const auto& query_type = request_data.at("type").AsString();
            if ( query_type == "Stop")
            {
                output.push_back(ReturnStop(request_data));
            }
            else if (query_type == "Bus")
            {
                output.push_back(ReturnBus(request_data));
            }
            else if (query_type == "Map")
            {
                std::stringstream stream;
                RenderMap(stream);
                output.emplace_back(json::Builder().StartDict().Key("map"s).Value(std::move(stream).str()).Key("request_id"s).Value(request_data.at("id").AsInt()).EndDict().Build());
            }
            else if (query_type == "Route")
            {
                output.push_back(ReturnRoute(request_data));
            }
        }
        return output;
    }

    void JsonReader::RenderMap(std::ostream& os)
    {
        render_ = std::make_unique<MapRenderer>(MapRenderer{std::move(db_.GetBusVector()), GetRenderSettings()});
        if (render_)
        {
            render_->Render(os);
        }
    }

    std::optional<JsonReader::Path> JsonReader::GetFilePath()
    {
        return file_path_;
    }

    void JsonReader::ParseSerializationSettings(const json::Dict& value)
    {
        auto iter = value.find("file");
        if (iter != value.end())
        {
            file_path_ = iter->second.AsString();
        }
    }

    void JsonReader::SetMapReanderSettings(MapRenderer::RenderSetting&& in)
    {
        render_settings_ = in;
    }

    void JsonReader::InitRouter(TransportCatalogue_Router::GraphBuilder::InitStruct&& graph_builder_init, Router::InitStruct&& router_init)
    {
        graph_builder_ = std::make_unique<TransportCatalogue_Router::GraphBuilder>(db_, std::forward<TransportCatalogue_Router::GraphBuilder::InitStruct>(graph_builder_init));
        router_ = std::make_unique<Router>(graph_builder_->GetGraphRef(), std::forward<Router::InitStruct>(router_init));
    }

    const TransportCatalogue_Router::GraphBuilder* JsonReader::GetGraphBuilderPtr() const
    {
        return graph_builder_.get();
    }

    const graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>* JsonReader::GetRouterPtr() const
    {
        return router_.get();
    }

    void JsonReader::SetRouterSettings(TransportCatalogue_Router::RouterSettings&& settings)
    {
        router_settings_ = settings;
    }

    const TransportCatalogue_Router::RouterSettings& JsonReader::GetRouterSettings() const
    {
        return router_settings_;
    }

} // end namespace NS_TransportCatalogue::Interfaces