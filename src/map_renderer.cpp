#include <map>
#include <optional>

#include "map_renderer.h"

namespace NS_TransportCatalogue::Interfaces
{
MapRenderer::MapRenderer(std::vector<const domain::Bus*> buses, const RenderSetting& settings): buses_(std::move(buses)), settings_(settings)
{
    std::sort(buses_.begin(), buses_.end(),
             [](const domain::Bus* lhs, const domain::Bus* rhs){return lhs->name < rhs->name;});
}

MapRenderer::Render_worker::Render_worker(const std::vector<const domain::Bus*>& buses_in, const RenderSetting& in_set): buses(buses_in), settings(in_set), projector([&]()
{
    std::vector<const geo::Coordinates*> stops_coordinates;

    for (const auto& bus : buses)
    {
        if (bus->stops.empty()) continue;

        for (const auto& stop : bus->stops)
        {
            stops_coordinates.push_back(&stop->coordinates);
            stops_point.emplace(stop->name, &stop->coordinates);
        }
        if (bus->root_type == domain::BussRootType::FORWARD)
        {
            for (auto stop = ++bus->stops.crbegin(); stop < bus->stops.crend(); ++stop)
            {
                stops_coordinates.push_back(&(*stop)->coordinates);
            }
        }
    }
    
    return domain::SphereProjector{stops_coordinates.begin(), stops_coordinates.end(), settings.width, settings.height, settings.padding};
}()) {}

void MapRenderer::Render_worker::RenderNameObj(std::vector<Obj_Name>& obj_con)
{
    for (auto& obj : obj_con)
    {
        svg_doc.Add(std::move(obj.first));
        svg_doc.Add(std::move(obj.second));
    }
}

void MapRenderer::Render_worker::LineRender(const domain::Bus& bus, svg::Color color)
{
    svg::Polyline bus_line;

    bool trig = false;
    for (const auto& stop: bus.stops)
    {
        if (!trig)
        {
            bus_names.push_back(CreateBusName(projector(stop->coordinates), bus.name, color));
            trig = true;
        }

        bus_line.AddPoint(projector(stop->coordinates));
    }
    if (bus.root_type == domain::BussRootType::FORWARD)
    {
        if (bus.stops.front()->coordinates != bus.stops.back()->coordinates)
        {
            bus_names.push_back(CreateBusName(projector((*bus.stops.crbegin())->coordinates), bus.name, color));
        }
        
        for (auto i = bus.stops.crbegin() + 1; i != bus.stops.crend(); ++i)
        {
            bus_line.AddPoint(projector((*i)->coordinates));
        }
    }

    bus_line.SetStrokeWidth(settings.line_width);
    bus_line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    bus_line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    bus_line.SetStrokeColor(std::move(color));
    bus_line.SetFillColor(svg::Color{"none"});

    svg_doc.Add(std::move(bus_line));
}

void MapRenderer::Render_worker::StopPointRender()
{

    for (auto&& [key, value] : stops_point)
    {
        svg::Point point(projector(*value));
        svg_doc.Add(std::move(CreateStopPoint(point)));
        stop_names.push_back(CreateStopName(point, std::string{key}));
    }
}

void MapRenderer::Render_worker::RunRender(std::ostream& os)
{
    auto color_iter = settings.color_palette.begin();

    for (const auto& bus : buses)
    {

        LineRender(*bus, *color_iter);

        ++color_iter;
        if (color_iter == settings.color_palette.end())
        {
            color_iter = settings.color_palette.begin();
        }
    }

    RenderNameObj(bus_names);
    StopPointRender();
    RenderNameObj(stop_names);

    svg_doc.Render(os);
}

void MapRenderer::Render(std::ostream& os) const
{
    Render_worker worker{buses_, settings_};
    worker.RunRender(os);
}

MapRenderer::Render_worker::Obj_Name MapRenderer::Render_worker::CreateBusName(svg::Point point, std::string name, svg::Color color) const
{
    svg::Text out_text;
    svg::Text out_back;
    out_text.SetPosition(point);
    out_text.SetOffset(settings.bus_label_offset);
    out_text.SetFontSize(static_cast<uint32_t>(settings.bus_label_font_size));
    out_text.SetFontFamily("Verdana");
    out_text.SetFontWeight("bold");
    out_text.SetFillColor(std::move(color));
    out_text.SetData(name);

    out_back.SetPosition(point);
    out_back.SetOffset(settings.bus_label_offset);
    out_back.SetFontSize(static_cast<uint32_t>(settings.bus_label_font_size));
    out_back.SetFontFamily("Verdana");
    out_back.SetFontWeight("bold");
    out_back.SetFillColor(settings.underlayer_color);
    out_back.SetStrokeColor(settings.underlayer_color);
    out_back.SetStrokeWidth(settings.underlayer_width);
    out_back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    out_back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
    out_back.SetData(std::move(name));

    return {std::move(out_back), std::move(out_text)};
}

svg::Circle MapRenderer::Render_worker::CreateStopPoint(svg::Point point) const
{
    svg::Circle out;
    out.SetCenter(point);
    out.SetRadius(settings.stop_radius);
    out.SetFillColor({"white"});

    return out;
}

MapRenderer::Render_worker::Obj_Name MapRenderer::Render_worker::CreateStopName(svg::Point point, std::string name) const
{
    svg::Text out_text;
    svg::Text out_back;

    out_text.SetPosition(point);
    out_text.SetOffset(settings.stop_label_offset);
    out_text.SetFontSize(static_cast<uint32_t>(settings.stop_label_font_size));
    out_text.SetFontFamily("Verdana");
    out_text.SetData(name);
    out_text.SetFillColor("black");

    out_back.SetPosition(point);
    out_back.SetOffset(settings.stop_label_offset);
    out_back.SetFontSize(static_cast<uint32_t>(settings.stop_label_font_size));
    out_back.SetFontFamily("Verdana");
    out_back.SetData(std::move(name));
    out_back.SetFillColor(settings.underlayer_color);
    out_back.SetStrokeColor(settings.underlayer_color);
    out_back.SetStrokeWidth(settings.underlayer_width);
    out_back.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
    out_back.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

    return {std::move(out_back), std::move(out_text)};
}
} // end namespace NS_TransportCatalogue::Interfaces