#include <cstdlib>

#include "map_renderer.h"

namespace map_renderer {
// ------------------------------- [Map Renderer] Realization -------------------------
//                                                                                    +
//                                                                                    + ------------------
// ------------------------------------------------------------------------------------ Sphere Projector +
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

// 
// 
//                                                                                    + ----------------------------
// ------------------------------------------------------------------------------------ Map Renderer [Auxiliaries] +

    const svg::Color MapRenderer::ChooseColor(const json::Node& to_process) const {
        if (to_process.IsString()) {
            return to_process.AsString();
        }
        else if (to_process.AsArray().size() == 3) {
            svg::Rgb to_add = { static_cast<std::uint8_t>(to_process.AsArray().front().AsInt()), 
                static_cast<std::uint8_t>(to_process.AsArray().at(1).AsInt()), 
                static_cast<std::uint8_t>(to_process.AsArray().back().AsInt())};
            return to_add;
        }

        svg::Rgba to_add = { static_cast<std::uint8_t>(to_process.AsArray().front().AsInt()),
            static_cast<std::uint8_t>(to_process.AsArray().at(1).AsInt()),
            static_cast<std::uint8_t>(to_process.AsArray().at(2).AsInt()),
            to_process.AsArray().back().AsDouble()
        };
        return to_add;
    }

// 
// 
//                                                                                    + ------------
// ------------------------------------------------------------------------------------ Filling in +

    void MapRenderer::HandleRenderRequests(const json::Document& json_document) {
        using namespace std::literals;

        const json::Dict& as_map = json_document.GetRoot().AsMap().at("render_settings"s).AsMap();

        width = as_map.at("width"s).AsDouble();
        height = as_map.at("height"s).AsDouble();
        padding = as_map.at("padding"s).AsDouble();
        line_width = as_map.at("line_width"s).AsDouble();
        stop_radius = as_map.at("stop_radius"s).AsDouble();
        bus_label_font_size = as_map.at("bus_label_font_size"s).AsInt();
        bus_label_offset = { as_map.at("bus_label_offset"s).AsArray().front().AsDouble(), as_map.at("bus_label_offset"s).AsArray().back().AsDouble() };
        stop_label_font_size = as_map.at("stop_label_font_size"s).AsInt();
        stop_label_offset = { as_map.at("stop_label_offset"s).AsArray().front().AsDouble(), as_map.at("stop_label_offset"s).AsArray().back().AsDouble() };
        underlayer_color = ChooseColor(as_map.at("underlayer_color"s));
        underlayer_width = as_map.at("underlayer_width"s).AsDouble();


        for (const json::Node& color : json_document.GetRoot().AsMap().at("render_settings"s).AsMap().at("color_palette"s).AsArray()) {
            color_palette_.push_back(ChooseColor(color));
        }

        std::map<std::string_view, std::pair<std::vector<std::string_view>, bool>> buses_and_stops;

        for (const auto& bus_or_stop : json_document.GetRoot().AsMap().at("base_requests"s).AsArray()) {
            const json::Dict& to_parse = bus_or_stop.AsMap();

            if (to_parse.at("type"s) == "Bus"s) {
                std::vector<std::string_view> proper_stops;

                if (to_parse.at("is_roundtrip"s).AsBool()) {
                    for (const auto& stop : to_parse.at("stops"s).AsArray()) {
                        proper_stops.push_back(stop.AsString());
                    }
                }
                else {
                    for (const auto& stop : to_parse.at("stops"s).AsArray()) {
                        proper_stops.push_back(stop.AsString());
                    }

                    for (auto it = to_parse.at("stops"s).AsArray().rbegin() + 1; it != to_parse.at("stops"s).AsArray().rend(); ++it) {
                        const std::string& stop = it->AsString();
                        proper_stops.push_back(stop);
                    }
                }

                buses_and_stops[to_parse.at("name"s).AsString()] = { proper_stops, to_parse.at("is_roundtrip"s).AsBool() };
                continue;
            }

            deque_stops_.push_back({ to_parse.at("name"s).AsString(), { to_parse.at("latitude"s).AsDouble(), to_parse.at("longitude"s).AsDouble() } });
        }

        for (const auto& [bus_name, proper_stops] : buses_and_stops) {
            domain::Bus bus_to_process({ std::string(bus_name), std::vector<domain::Stop*>{} });

            for (const auto& proper_stop : proper_stops.first) {
                bus_to_process.stops_with_duplicates.push_back(&*std::find(deque_stops_.begin(), deque_stops_.end(), proper_stop));
                sorted_stops_.insert(proper_stop);
            }

            routes_.push_back({ bus_to_process, proper_stops.second });
        }
    }

// 
// 
//                                                                                    + -----------------
// ------------------------------------------------------------------------------------ Lines rendering +

    void MapRenderer::HandleLines(const SphereProjector& sphere_projector) {
        using namespace std::literals;

        std::size_t color_to_be_applied = 0;
        for (auto route = routes_.begin(); route != routes_.end(); ++route, ++color_to_be_applied) {
            auto current_color = color_to_be_applied % color_palette_.size();

            if (route->first.stops_with_duplicates.size()) {
                std::vector<geo::Coordinates> geo_coordinates;

                for (const auto& stop : route->first.stops_with_duplicates) {
                    geo_coordinates.push_back(stop->coordinates);
                }

                svg::Polyline to_be_added;
                to_be_added.SetFillColor(svg::NoneColor).SetStrokeWidth(line_width);
                to_be_added.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                to_be_added.SetStrokeColor(color_palette_.at(current_color));

                for (const auto& coordinate : geo_coordinates) {
                    const svg::Point screen_coordinate = sphere_projector(coordinate);
                    to_be_added.AddPoint(screen_coordinate);
                }

                svgs_to_be_rendered_.Add(to_be_added);
            }
        }
    }

// 
// 
//                                                                                    + ---------------------
// ------------------------------------------------------------------------------------ Line text rendering +

    void MapRenderer::HandleLineText(const SphereProjector& sphere_projector) {
        using namespace std::literals;

        std::size_t color_to_be_applied = 0;
        for (auto route = routes_.begin(); route != routes_.end(); ++route, ++color_to_be_applied) {
            auto current_color = color_to_be_applied % color_palette_.size();

            if (route->first.stops_with_duplicates.size()) {
                if (!route->second && route->first.stops_with_duplicates.front()->name != route->first.stops_with_duplicates.at(route->first.stops_with_duplicates.size() / 2)->name) {
                    svg::Point screen_coordinate = sphere_projector(route->first.stops_with_duplicates.front()->coordinates);

                    svg::Text begin_underlayer_to_be_added;
                    begin_underlayer_to_be_added.SetOffset(bus_label_offset);
                    begin_underlayer_to_be_added.SetFontSize(bus_label_font_size);
                    begin_underlayer_to_be_added.SetFontFamily("Verdana"s);
                    begin_underlayer_to_be_added.SetFontWeight("bold"s);
                    begin_underlayer_to_be_added.SetData(route->first.name);

                    svg::Text begin_text_to_be_added = begin_underlayer_to_be_added;

                    begin_underlayer_to_be_added.SetFillColor(underlayer_color);
                    begin_underlayer_to_be_added.SetStrokeColor(underlayer_color);
                    begin_underlayer_to_be_added.SetStrokeWidth(underlayer_width);
                    begin_underlayer_to_be_added.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                    begin_text_to_be_added.SetFillColor(color_palette_.at(current_color));
                    svg::Text end_underlayer_to_be_added = begin_underlayer_to_be_added;
                    svg::Text end_text_to_be_added = begin_text_to_be_added;

                    begin_underlayer_to_be_added.SetPosition(screen_coordinate);
                    begin_text_to_be_added.SetPosition(screen_coordinate);

                    screen_coordinate = sphere_projector(route->first.stops_with_duplicates.at(route->first.stops_with_duplicates.size() / 2)->coordinates);
                    end_underlayer_to_be_added.SetPosition(screen_coordinate);
                    end_text_to_be_added.SetPosition(screen_coordinate);

                    svgs_to_be_rendered_.Add(begin_underlayer_to_be_added);
                    svgs_to_be_rendered_.Add(begin_text_to_be_added);
                    svgs_to_be_rendered_.Add(end_underlayer_to_be_added);
                    svgs_to_be_rendered_.Add(end_text_to_be_added);

                    continue;
                }

                const svg::Point screen_coordinate = sphere_projector(route->first.stops_with_duplicates.front()->coordinates);

                svg::Text underlayer_to_be_added;
                underlayer_to_be_added.SetPosition(screen_coordinate);
                underlayer_to_be_added.SetOffset(bus_label_offset);
                underlayer_to_be_added.SetFontSize(bus_label_font_size);
                underlayer_to_be_added.SetFontFamily("Verdana"s);
                underlayer_to_be_added.SetFontWeight("bold"s);
                underlayer_to_be_added.SetData(route->first.name);

                svg::Text text_to_be_added = underlayer_to_be_added;

                underlayer_to_be_added.SetFillColor(underlayer_color);
                underlayer_to_be_added.SetStrokeColor(underlayer_color);
                underlayer_to_be_added.SetStrokeWidth(underlayer_width);
                underlayer_to_be_added.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                text_to_be_added.SetFillColor(color_palette_.at(current_color));

                svgs_to_be_rendered_.Add(underlayer_to_be_added);
                svgs_to_be_rendered_.Add(text_to_be_added);
            }
        }
    }

// 
// 
//                                                                                    + ------------------
// ------------------------------------------------------------------------------------ Circle rendering +

    void MapRenderer::HandleCircles(const SphereProjector& sphere_projector) {
        using namespace std::literals;

        for (const auto& stop : sorted_stops_) {
            const svg::Point screen_coordinate = sphere_projector(std::find(deque_stops_.begin(), deque_stops_.end(), stop)->coordinates);

            svg::Circle to_be_added;
            to_be_added.SetCenter(screen_coordinate);
            to_be_added.SetRadius(stop_radius);
            to_be_added.SetFillColor("white"s);

            svgs_to_be_rendered_.Add(to_be_added);
        }
    }

// 
// 
//                                                                                    + -----------------------
// ------------------------------------------------------------------------------------ Circle text rendering +

    void MapRenderer::HandleCircleText(const SphereProjector& sphere_projector) {
        using namespace std::literals;

        for (const auto& stop : sorted_stops_) {
            domain::Stop& stop_to_process = *std::find(deque_stops_.begin(), deque_stops_.end(), stop);
            const svg::Point screen_coordinate = sphere_projector(stop_to_process.coordinates);

            svg::Text underlayer_to_be_added;
            underlayer_to_be_added.SetPosition(screen_coordinate);
            underlayer_to_be_added.SetOffset(stop_label_offset);
            underlayer_to_be_added.SetFontSize(stop_label_font_size);
            underlayer_to_be_added.SetFontFamily("Verdana"s);
            underlayer_to_be_added.SetData(stop_to_process.name);

            svg::Text text_to_be_added = underlayer_to_be_added;

            underlayer_to_be_added.SetFillColor(underlayer_color);
            underlayer_to_be_added.SetStrokeColor(underlayer_color);
            underlayer_to_be_added.SetStrokeWidth(underlayer_width);
            underlayer_to_be_added.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            text_to_be_added.SetFillColor("black"s);

            svgs_to_be_rendered_.Add(underlayer_to_be_added);
            svgs_to_be_rendered_.Add(text_to_be_added);
        }
    }

// 
// 
//                                                                                    + ------------------
// ------------------------------------------------------------------------------------ Rendering Facade +

    svg::Document MapRenderer::CreateMap() {
        using namespace std::literals;

        std::vector<geo::Coordinates> each_geo_coordinate;
        for (const auto& route : routes_) {
            for (const auto& stop : route.first.stops_with_duplicates) {
                each_geo_coordinate.push_back(stop->coordinates);
            }
        }

        SphereProjector sphere_projector(each_geo_coordinate.begin(), each_geo_coordinate.end(), width, height, padding);

        HandleLines(sphere_projector);
        HandleLineText(sphere_projector);
        HandleCircles(sphere_projector);
        HandleCircleText(sphere_projector);

        return std::move(svgs_to_be_rendered_);
    }
} // namespace map_renderer