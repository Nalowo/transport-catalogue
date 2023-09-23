#include "serialization.h"

namespace NS_TransportCatalogue::Serealization_Worker
{

// ======Class Serealization===========

Serealization::Serealization(const NS_TransportCatalogue::TransportCatalogue& catalog): NS_TransportCatalogue::DB_Worker(catalog), fields_(GetSerealizFields()) {}

void Serealization::RunSerealization(std::ostream& output)
{
    CreateProtoTransportCatalogue().SerializePartialToOstream(&output);
}

transport_catalogue_serialize::TransportCatalogue Serealization::CreateProtoTransportCatalogue() const
{
    transport_catalogue_serialize::TransportCatalogue out;

    auto stop_iter = fields_.stops_base_.cbegin();
    auto bus_iter = fields_.bus_base_.cbegin();
    auto length_iter = fields_.length_stop_to_neighbor_.cbegin();

    for (size_t i = 0; i < std::max({fields_.stops_base_.size(), fields_.bus_base_.size(), fields_.length_stop_to_neighbor_.size()}); ++i)
    {
        if (i < fields_.stops_base_.size())
        {

            *out.add_stops() = CreateProtoStop(*stop_iter);

            ++stop_iter;
        }

        if (i < fields_.bus_base_.size())
        {

            *out.add_buses() = CreateProtoBus(*bus_iter);

            ++bus_iter;
        }

        if (i < fields_.length_stop_to_neighbor_.size())
        {

            *out.add_length_btw_stops() = CreateProtoStopToStop(length_iter->first.first, length_iter->first.second, length_iter->second);

            ++length_iter;
        }
    }

    if (map_settings_)
    {
        *out.mutable_render_settings() = CreateProtoRenderSettings();
    }

    if (graph_builder_ptr_ && router_ptr_)
    {
        *out.mutable_router_settings() = CreateProtoRouterSettings(*router_settings_);
        *out.mutable_route_builder() = CreateProtoGraphBuilder();
        *out.mutable_router() = CreateProtoRouter();
    }

    return out;
}

transport_catalogue_serialize::Stop Serealization::CreateProtoStop(const domain::Stop& stop) const
{
    transport_catalogue_serialize::Stop out;

    out.set_id(stop.id);
    out.set_name(stop.name);

    transport_catalogue_serialize::Geo coordinates;

    coordinates.set_lat(stop.coordinates.lat);
    coordinates.set_lng(stop.coordinates.lng);

    *out.mutable_coordinates() = coordinates;

    return out;
}

transport_catalogue_serialize::Bus Serealization::CreateProtoBus(const domain::Bus& bus) const
{
    transport_catalogue_serialize::Bus out;

    out.set_id(bus.id);
    out.set_name(bus.name);
    
    for (const auto& stop : bus.stops)
    {
        out.add_stops(stop->id);
    }

    if (bus.root_type == domain::BussRootType::CYCLE)
    {
        out.set_root_type(transport_catalogue_serialize::Root_Type::CYCLE);
    }
    else
    {
        out.set_root_type(transport_catalogue_serialize::Root_Type::FORWARD);
    }

    return out;
}

transport_catalogue_serialize::StopToStop Serealization::CreateProtoStopToStop(const domain::Stop* from, const domain::Stop* to, unsigned int lenght) const
{
    transport_catalogue_serialize::StopToStop out;

    out.set_id_from(from->id);
    out.set_id_to(to->id);
    out.set_length(lenght);

    return out;
}

transport_catalogue_serialize::RenderSettings Serealization::CreateProtoRenderSettings() const
{
    transport_catalogue_serialize::RenderSettings out;

    out.set_width(map_settings_->width);
    out.set_height(map_settings_->height);
    out.set_padding(map_settings_->padding);
    out.set_line_width(map_settings_->line_width);
    out.set_stop_radius(map_settings_->stop_radius);
    out.set_bus_label_font_size(map_settings_->bus_label_font_size);

    transport_catalogue_serialize::Point point1;

    point1.set_x(map_settings_->bus_label_offset.x);
    point1.set_y(map_settings_->bus_label_offset.y);
    *out.mutable_bus_label_offset() = point1;

    out.set_stop_label_font_size(map_settings_->stop_label_font_size);
    
    transport_catalogue_serialize::Point point2;

    point2.set_x(map_settings_->stop_label_offset.x);
    point2.set_y(map_settings_->stop_label_offset.y);
    *out.mutable_stop_label_offset() = point2;


    *out.mutable_underlayer_color() = CreateProtoColor(map_settings_->underlayer_color);
    out.set_underlayer_width(map_settings_->underlayer_width);

    for (const auto& color : map_settings_->color_palette)
    {
        *out.add_color_palette() = CreateProtoColor(color);
    }

    return out;
}

transport_catalogue_serialize::Color Serealization::CreateProtoColor(const svg::Color& color_in) const
{
    transport_catalogue_serialize::Color out;

        if (std::holds_alternative<std::monostate>(color_in))
        {
            *out.mutable_string_color() = std::string();
        }
        else if (std::holds_alternative<std::string>(color_in))
        {
            *out.mutable_string_color() = std::get<std::string>(color_in);
        }
        else if (std::holds_alternative<svg::Rgb>(color_in))
        {
            transport_catalogue_serialize::Color_Rgb rgb;

            auto& rgb_in = std::get<svg::Rgb>(color_in);

            rgb.set_red(rgb_in.red);
            rgb.set_green(rgb_in.green);
            rgb.set_blue(rgb_in.blue);

            *out.mutable_rgb() = rgb;
        }
        else if (std::holds_alternative<svg::Rgba>(color_in))
        {
            transport_catalogue_serialize::Color_Rgba rgba;

            auto& rgba_in = std::get<svg::Rgba>(color_in);

            rgba.set_red(rgba_in.red);
            rgba.set_green(rgba_in.green);
            rgba.set_blue(rgba_in.blue);
            rgba.set_opacity(rgba_in.opacity);

            *out.mutable_rgba() = rgba;
        }

        return out;
}

void Serealization::SetMapSettings(const Interfaces::MapRenderer::RenderSetting* settings)
{
    map_settings_ = settings;
}

void Serealization::SetRouterSettings(const TransportCatalogue_Router::RouterSettings* settings)
{
    router_settings_ = settings;
}

void Serealization::SetGraphBuilder(const TransportCatalogue_Router::GraphBuilder* builder)
{
    graph_builder_ptr_ = builder;
}
void Serealization::SetRouter(const graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>* router)
{
    router_ptr_ = router;
}

transport_catalogue_serialize::RouterSettings Serealization::CreateProtoRouterSettings(TransportCatalogue_Router::RouterSettings settings) const
{
    transport_catalogue_serialize::RouterSettings out;

    out.set_bus_speed(settings.bus_speed);
    out.set_bus_wait_time(settings.bus_wait_time);

    return out;
}

transport_catalogue_serialize::DirectedWeightedGraph Serealization::CreateProtoDWGraph(const DWGraph& graph) const
{
    transport_catalogue_serialize::DirectedWeightedGraph out;

    auto graph_data = graph.GetData();

    auto edge_iter = graph_data.edges.cbegin();
    auto incid_list_iter = graph_data.incident_lists.cbegin();
    for (size_t i = 0; i < std::max({graph_data.edges.size(), graph_data.incident_lists.size()}); ++i)
    {
        if (i < graph_data.edges.size())
        {
            transport_catalogue_serialize::DirectedWeightedGraph::Edge edge_in;
            edge_in.set_from(edge_iter->from);
            edge_in.set_to(edge_iter->to);
            edge_in.set_weight(edge_iter->weight.weight);

            *out.add_edges() = edge_in;

            ++edge_iter;
        }

        if (i < graph_data.incident_lists.size())
        {
            transport_catalogue_serialize::DirectedWeightedGraph::IncidenceList il_proto;
            for (const auto il : *incid_list_iter)
            {
                il_proto.add_edge_id_list(il);
            }

            *out.add_incidence_lists() = std::move(il_proto);

            ++incid_list_iter;
        }
    }

    return out;
}

transport_catalogue_serialize::GraphBuilder Serealization::CreateProtoGraphBuilder() const
{
    transport_catalogue_serialize::GraphBuilder out;

    auto graph_builder_data = graph_builder_ptr_->GetData();

    *out.mutable_graph() = CreateProtoDWGraph(graph_builder_data.graph);

    for (const auto& edge_id : graph_builder_data.edgels)
    {
        transport_catalogue_serialize::GraphBuilder::EdgeID proto_edge_id;
        
        proto_edge_id.set_name(edge_id.name.data());
        proto_edge_id.set_span_count(edge_id.span_count);
        proto_edge_id.set_weight(edge_id.weight);

        *out.add_edgels() = std::move(proto_edge_id);
    }

    return out;
}

transport_catalogue_serialize::Router Serealization::CreateProtoRouter() const
{
    transport_catalogue_serialize::Router out;

    auto router_data = router_ptr_->GetData();

    for (const auto& elemtnt_extermal_arr : router_data.routes_internal_data)
    {
        transport_catalogue_serialize::Router::RouterIternalData proto_rout_data;

        for (const auto& elemtnt_iternal_arr : elemtnt_extermal_arr)
        {
            transport_catalogue_serialize::Router::RouterIternalData::OptRouterData opt_data;
            
            if (elemtnt_iternal_arr)
            {
                transport_catalogue_serialize::Router::RouterIternalData::OptRouterData::RouterData data;

                data.set_weight(elemtnt_iternal_arr->weight.weight);

                if (elemtnt_iternal_arr->prev_edge)
                {
                    transport_catalogue_serialize::Router::RouterIternalData::OptRouterData::RouterData::Prev_Edge prev_edge;
                    prev_edge.set_data(*elemtnt_iternal_arr->prev_edge);
                    
                    *data.mutable_prev_edge() = prev_edge;
                }

                *opt_data.mutable_data() = data;
            }

            *proto_rout_data.add_data_list() = opt_data;
        }

        *out.add_routes_data() = proto_rout_data;
    }

    return out;
}

// ======Class Serealization===========







// ======Class Deserealization=========

Deserealization::Deserealization(NS_TransportCatalogue::TransportCatalogue& catalog): DB_Worker(catalog), fields_(GetDeserealizFields()) {}

void Deserealization::RunDeserealization(std::istream& input)
{
    DeserealizationTC(input);
    FeedTCFieds();
}

void Deserealization::RunDeserealization(std::istream& input, Interfaces::JsonReader& reader)
{
    RunDeserealization(input);

    if (desed_catalog_.has_render_settings())
    {
        reader.SetMapReanderSettings(CreateMapRenderSettings(*desed_catalog_.mutable_render_settings()));
    }
    
    if (desed_catalog_.has_route_builder() && desed_catalog_.has_router())
    {
        reader.SetRouterSettings(CreateRouterSettings(desed_catalog_.mutable_router_settings()));
        reader.InitRouter(CreateGraphBuilderInit(CreateRouterSettings(desed_catalog_.mutable_router_settings()), desed_catalog_.mutable_route_builder()), CreateRouterInit(desed_catalog_.mutable_router()));
    }
}

void Deserealization::DeserealizationTC(std::istream& input)
{
    desed_catalog_.ParseFromIstream(&input);
}

void Deserealization::FeedTCFieds()
{
    FeedFields(desed_catalog_.mutable_buses(), desed_catalog_.mutable_length_btw_stops(), FeedStops(desed_catalog_.mutable_stops()));
}

std::vector<domain::Stop*> Deserealization::FeedStops(google::protobuf::RepeatedPtrField<transport_catalogue_serialize::Stop>* stops_arr)
{
    std::vector<domain::Stop*> out(stops_arr->size());

    fields_.stops_index_table_.reserve(stops_arr->size());
    fields_.bus_throw_stop_.reserve(stops_arr->size());

    for (int i = 0; i <  stops_arr->size(); ++i)
    {
        domain::Stop stop
        {
            std::move(*stops_arr->Mutable(i)->mutable_name()),
            {stops_arr->Mutable(i)->coordinates().lat(), stops_arr->Mutable(i)->coordinates().lng()},
        };
        stop.id = stops_arr->Mutable(i)->id();

        fields_.stops_base_.push_back(std::move(stop));
        
        if (fields_.stops_base_.back().id > out.size())
        {
            out.resize(fields_.stops_base_.back().id);
        }

        fields_.stops_index_table_[fields_.stops_base_.back().name] = &fields_.stops_base_.back();

        out[fields_.stops_base_.back().id] = &fields_.stops_base_.back();
    }

    return out;
}

void Deserealization::FeedBus(transport_catalogue_serialize::Bus* bus_in, const std::vector<domain::Stop*>& stops_id_index)
{

        domain::Bus& bus = fields_.bus_base_.emplace_back();

        bus.name =  std::move(*bus_in->mutable_name());
        bus.id = bus_in->id();

        if (bus_in->root_type() == transport_catalogue_serialize::Root_Type::CYCLE)
        {
            bus.root_type = domain::BussRootType::CYCLE;
        } 
        else if (bus_in->root_type() == transport_catalogue_serialize::Root_Type::FORWARD)
        {
            bus.root_type = domain::BussRootType::FORWARD;
        }

        bus.stops.reserve(bus_in->stops_size());
        for (int is = 0; is < bus_in->stops_size(); ++is)
        {
            auto stop_id = bus_in->stops(is);
            bus.stops.push_back(stops_id_index[stop_id]);
            bus.distance += bus.stops.size() > 1 ? geo::ComputeDistance((*(bus.stops.end() - 2))->coordinates, (*(bus.stops.end() - 1))->coordinates) : 0;

            fields_.bus_throw_stop_[bus.stops.back()].insert(bus.name);
        }

        fields_.bus_index_table_[fields_.bus_base_.back().name] = &fields_.bus_base_.back();
}

void Deserealization::FeedLength(transport_catalogue_serialize::StopToStop* proto_st_to_st, const std::vector<domain::Stop*>& stops_id_index)
{
    fields_.length_stop_to_neighbor_.insert(
        std::make_pair(
            std::make_pair(stops_id_index[proto_st_to_st->id_from()], stops_id_index[proto_st_to_st->id_to()]),
            proto_st_to_st->length()
        )
    );
}

void Deserealization::FeedFields(google::protobuf::RepeatedPtrField<transport_catalogue_serialize::Bus>* bus_arr, google::protobuf::RepeatedPtrField<transport_catalogue_serialize::StopToStop>* length_arr, std::vector<domain::Stop*>&& stops_id_index)
{
    fields_.length_stop_to_neighbor_.reserve(length_arr->size());
    fields_.bus_index_table_.reserve(bus_arr->size());

    for (int i = 0; i < std::max({bus_arr->size(), length_arr->size()}); ++i)
    {
        if (i < bus_arr->size())
        {
            FeedBus(bus_arr->Mutable(i), stops_id_index);
        }

        if (i < length_arr->size())
        {
            FeedLength(length_arr->Mutable(i), stops_id_index);
        }
    }
}

Interfaces::MapRenderer::RenderSetting Deserealization::CreateMapRenderSettings(transport_catalogue_serialize::RenderSettings& settings)
{
    Interfaces::MapRenderer::RenderSetting render_settings;

    render_settings.width = settings.width();
    render_settings.height = settings.height();
    render_settings.padding = settings.padding();
    render_settings.line_width = settings.line_width();
    render_settings.stop_radius = settings.stop_radius();
    render_settings.bus_label_font_size = settings.bus_label_font_size();
    render_settings.bus_label_offset = {settings.bus_label_offset().x(), settings.bus_label_offset().y()};
    render_settings.stop_label_font_size = settings.stop_label_font_size();
    render_settings.stop_label_offset = {settings.stop_label_offset().x(), settings.stop_label_offset().y()};

    render_settings.underlayer_color = CreateColor(*settings.mutable_underlayer_color());
    render_settings.underlayer_width = settings.underlayer_width();


    for (auto& color : *settings.mutable_color_palette())
    {
        render_settings.color_palette.push_back(CreateColor(color));
    }

    return render_settings;
}

svg::Color Deserealization::CreateColor(transport_catalogue_serialize::Color& color)
{
    svg::Color out;

    auto cast = [](uint32_t in)
    {
        return static_cast<uint8_t>(in);
    };

    if (color.color_case() == transport_catalogue_serialize::Color::ColorCase::kStringColor)
    {
        if (color.string_color().empty())
        {
            out = std::monostate{};
        }
        else
        {
            out = std::move(*color.mutable_string_color());
        }
    }
    else if (color.color_case() == transport_catalogue_serialize::Color::ColorCase::kRgb)
    {
        auto rgb = color.rgb();
        out = svg::Rgb{cast(rgb.red()), cast(rgb.green()), cast(rgb.blue())};
    }
    else if (color.color_case() == transport_catalogue_serialize::Color::ColorCase::kRgba)
    {
        auto rgba = color.rgba();
        out = svg::Rgba{cast(rgba.red()), cast(rgba.green()), cast(rgba.blue()), rgba.opacity()};
    }

    return out;
}

TransportCatalogue_Router::RouterSettings Deserealization::CreateRouterSettings(transport_catalogue_serialize::RouterSettings* settings)
{
    TransportCatalogue_Router::RouterSettings out;

    out.bus_speed = settings->bus_speed();
    out.bus_wait_time = settings->bus_wait_time();

    return out;
}

graph::DirectedWeightedGraph<TransportCatalogue_Router::GraphBuilder::RouterWeight>::InitStruct Deserealization::CreateDWGraphInit(transport_catalogue_serialize::DirectedWeightedGraph* proto_graph)
{
    graph::DirectedWeightedGraph<TransportCatalogue_Router::GraphBuilder::RouterWeight>::InitStruct out;

    for (int i = 0; i < std::max({proto_graph->edges_size(), proto_graph->incidence_lists_size()}); ++i)
    {
        if ( i < proto_graph->edges_size())
        {
            graph::Edge<TransportCatalogue_Router::GraphBuilder::RouterWeight> edge_in;

            auto& proto_edge = proto_graph->edges(i);

            edge_in.from = proto_edge.from();
            edge_in.to = proto_edge.to();
            edge_in.weight.weight = proto_edge.weight();

            out.edges.push_back(std::move(edge_in));

        }

        if (i < proto_graph->incidence_lists_size())
        {
            out.incidence_lists.emplace_back();

            for (auto value : proto_graph->incidence_lists(i).edge_id_list())
            {
                out.incidence_lists.back().push_back(value);
            }
        }
    }

    return out;
}

TransportCatalogue_Router::GraphBuilder::InitStruct Deserealization::CreateGraphBuilderInit(TransportCatalogue_Router::RouterSettings&& settings, transport_catalogue_serialize::GraphBuilder* proto_puilder)
{
    TransportCatalogue_Router::GraphBuilder::InitStruct out;

    out.settings = settings;
    out.graph = CreateDWGraphInit(proto_puilder->mutable_graph());

    for (auto& value : *proto_puilder->mutable_edgels())
    {
        TransportCatalogue_Router::GraphBuilder::EdgeID edge_id_in;

        edge_id_in.name = std::move(*value.mutable_name());
        edge_id_in.span_count = value.span_count();
        edge_id_in.weight = value.weight();

        out.edgels.push_back(std::move(edge_id_in));
    }

    return out;
}

graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>::InitStruct Deserealization::CreateRouterInit(transport_catalogue_serialize::Router* proto_router)
{
    graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>::InitStruct out;

    for (auto& value_d1 : *proto_router->mutable_routes_data())
    {
        out.routes_internal_data.emplace_back();

        for (auto& value_d2 : *value_d1.mutable_data_list())
        {
            if (value_d2.has_data())
            {
                if (value_d2.data().has_prev_edge())
                {
                    graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>::RouteInternalData data_in{value_d2.data().weight(), value_d2.data().prev_edge().data()};
                    out.routes_internal_data.back().push_back(data_in);
                }
                else
                {
                    graph::Router<TransportCatalogue_Router::GraphBuilder::RouterWeight>::RouteInternalData data_in{value_d2.data().weight(), std::nullopt};
                    out.routes_internal_data.back().push_back(data_in);
                }
            }
            else
            {
                out.routes_internal_data.back().emplace_back(std::nullopt);
            }
        }
    }

    return out;
}

// ======Class Deserealization=========

} // end namespace Serealization_Worker