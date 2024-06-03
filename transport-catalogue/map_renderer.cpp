#include <cstdlib>
#include <utility>

#include "map_renderer.h"

namespace map_renderer {
// ------------ [Map Renderer] Realization ------------
//                                                    +
//                                                    + ------------------
// ---------------------------------------------------- Sphere Projector +

    namespace detail {
        bool IsZero(double value) {
            return std::abs(value) < EPSILON;
        }

        svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }
    } // namespace detail

// 
// 
//                                                    + ----------------------
// ---------------------------------------------------- Map Renderer Setters +

    void MapRenderer::SetSettings(Settings&& settings) {
        settings_ = std::move(settings);
    }

    void MapRenderer::SetColorPalette(std::vector<svg::Color>&& color_palette) {
        color_palette_ = std::move(color_palette);
    }

    void MapRenderer::SetDequeStops(std::deque<domain::Stop>&& deque_stops) {
        deque_stops_ = std::move(deque_stops);
    }

    void MapRenderer::SetSortedStops(std::set<std::string_view>&& sorted_stops) {
        sorted_stops_ = std::move(sorted_stops);
    }

    void MapRenderer::SetRoutes(std::deque<std::pair<domain::Bus, bool>>&& routes) {
        routes_ = std::move(routes);
    }

// 
// 
//                                                    + ----------------
// ---------------------------------------------------- Lines creating +

    void MapRenderer::RenderLines(const detail::SphereProjector& sphere_projector) {
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
                to_be_added.SetFillColor(svg::NoneColor).SetStrokeWidth(settings_.line_width);
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
//                                                    + --------------------
// ---------------------------------------------------- Line text creating +

    void MapRenderer::RenderLineText(const detail::SphereProjector& sphere_projector) {
        using namespace std::literals;

        std::size_t color_to_be_applied = 0;
        for (auto route = routes_.begin(); route != routes_.end(); ++route, ++color_to_be_applied) {
            auto current_color = color_to_be_applied % color_palette_.size();

            if (route->first.stops_with_duplicates.size()) {
                if (!route->second && route->first.stops_with_duplicates.front()->name != route->first.stops_with_duplicates.at(route->first.stops_with_duplicates.size() / 2)->name) {
                    svg::Point screen_coordinate = sphere_projector(route->first.stops_with_duplicates.front()->coordinates);

                    svg::Text begin_underlayer_to_be_added;
                    begin_underlayer_to_be_added.SetOffset(settings_.bus_label_offset);
                    begin_underlayer_to_be_added.SetFontSize(settings_.bus_label_font_size);
                    begin_underlayer_to_be_added.SetFontFamily("Verdana"s);
                    begin_underlayer_to_be_added.SetFontWeight("bold"s);
                    begin_underlayer_to_be_added.SetData(route->first.name);

                    svg::Text begin_text_to_be_added = begin_underlayer_to_be_added;

                    begin_underlayer_to_be_added.SetFillColor(settings_.underlayer_color);
                    begin_underlayer_to_be_added.SetStrokeColor(settings_.underlayer_color);
                    begin_underlayer_to_be_added.SetStrokeWidth(settings_.underlayer_width);
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
                underlayer_to_be_added.SetOffset(settings_.bus_label_offset);
                underlayer_to_be_added.SetFontSize(settings_.bus_label_font_size);
                underlayer_to_be_added.SetFontFamily("Verdana"s);
                underlayer_to_be_added.SetFontWeight("bold"s);
                underlayer_to_be_added.SetData(route->first.name);

                svg::Text text_to_be_added = underlayer_to_be_added;

                underlayer_to_be_added.SetFillColor(settings_.underlayer_color);
                underlayer_to_be_added.SetStrokeColor(settings_.underlayer_color);
                underlayer_to_be_added.SetStrokeWidth(settings_.underlayer_width);
                underlayer_to_be_added.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                text_to_be_added.SetFillColor(color_palette_.at(current_color));

                svgs_to_be_rendered_.Add(underlayer_to_be_added);
                svgs_to_be_rendered_.Add(text_to_be_added);
            }
        }
    }

// 
// 
//                                                    + -----------------
// ---------------------------------------------------- Circle creating +

    void MapRenderer::RenderCircles(const detail::SphereProjector& sphere_projector) {
        using namespace std::literals;

        for (const auto& stop : sorted_stops_) {
            const svg::Point screen_coordinate = sphere_projector(std::find(deque_stops_.begin(), deque_stops_.end(), stop)->coordinates);

            svg::Circle to_be_added;
            to_be_added.SetCenter(screen_coordinate);
            to_be_added.SetRadius(settings_.stop_radius);
            to_be_added.SetFillColor("white"s);

            svgs_to_be_rendered_.Add(to_be_added);
        }
    }

// 
// 
//                                                    + ----------------------
// ---------------------------------------------------- Circle text creating +

    void MapRenderer::RenderCircleText(const detail::SphereProjector& sphere_projector) {
        using namespace std::literals;

        for (const auto& stop : sorted_stops_) {
            domain::Stop& stop_to_process = *std::find(deque_stops_.begin(), deque_stops_.end(), stop);
            const svg::Point screen_coordinate = sphere_projector(stop_to_process.coordinates);

            svg::Text underlayer_to_be_added;
            underlayer_to_be_added.SetPosition(screen_coordinate);
            underlayer_to_be_added.SetOffset(settings_.stop_label_offset);
            underlayer_to_be_added.SetFontSize(settings_.stop_label_font_size);
            underlayer_to_be_added.SetFontFamily("Verdana"s);
            underlayer_to_be_added.SetData(stop_to_process.name);

            svg::Text text_to_be_added = underlayer_to_be_added;

            underlayer_to_be_added.SetFillColor(settings_.underlayer_color);
            underlayer_to_be_added.SetStrokeColor(settings_.underlayer_color);
            underlayer_to_be_added.SetStrokeWidth(settings_.underlayer_width);
            underlayer_to_be_added.SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            text_to_be_added.SetFillColor("black"s);

            svgs_to_be_rendered_.Add(underlayer_to_be_added);
            svgs_to_be_rendered_.Add(text_to_be_added);
        }
    }

// 
// 
//                                                    + ------------------
// ---------------------------------------------------- Rendering Facade +

    svg::Document MapRenderer::RenderMap() {
        std::vector<geo::Coordinates> each_geo_coordinate;
        for (const auto& route : routes_) {
            for (const auto& stop : route.first.stops_with_duplicates) {
                each_geo_coordinate.push_back(stop->coordinates);
            }
        }

        detail::SphereProjector sphere_projector(each_geo_coordinate.begin(), each_geo_coordinate.end(), settings_.width, settings_.height, settings_.padding);

        RenderLines(sphere_projector);
        RenderLineText(sphere_projector);
        RenderCircles(sphere_projector);
        RenderCircleText(sphere_projector);

        return std::move(svgs_to_be_rendered_);
    }
} // namespace map_renderer