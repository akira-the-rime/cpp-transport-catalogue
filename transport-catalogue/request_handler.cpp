#include <algorithm>
#include <sstream>

#include "request_handler.h"

namespace request_handler {
// ------------------------------- [Request Handler] Realization ----------------------
//                                                                                    +
//                                                                                    + ------------
// ------------------------------------------------------------------------------------ Constuctor +

	RequestHandler::RequestHandler(catalogue::TransportCatalogue& database) 
		: database_(database)
		, renderer_(nullptr) {
	}

	RequestHandler::RequestHandler(catalogue::TransportCatalogue& database, map_renderer::MapRenderer& renderer)
		: database_(database)
		, renderer_(&renderer) {
	}

// 
// 
//                                                                                    + ------------
// ------------------------------------------------------------------------------------ Filling in +

	void RequestHandler::FillTransportCatalogue(const json::Document& document) {
		using namespace std::literals;

		std::unordered_map<std::string_view, json::Dict> stops_and_destinations;
		std::unordered_map<std::string_view, std::vector<std::string_view>> buses;

		for (const auto& bus_or_stop : document.GetRoot().AsMap().at("base_requests"s).AsArray()) {
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

				buses[to_parse.at("name"s).AsString()] = proper_stops;
				continue;
			}

			database_.AddStop(to_parse.at("name"s).AsString(), { to_parse.at("latitude"s).AsDouble(), to_parse.at("longitude"s).AsDouble() });

			if (to_parse.contains("road_distances"s)) {
				stops_and_destinations[to_parse.at("name"s).AsString()] =  to_parse.at("road_distances"s).AsMap();
			}
		}

		for (const auto& [stop, destinations] : stops_and_destinations) {
			for (const auto& [destination, length] : destinations) {
				database_.AddDestination(std::string(stop), destination, length.AsInt());
			}
		}

		for (const auto& [name, proper_stops] : buses) {
			database_.AddBus(std::string(name), proper_stops);
		}
	}

// 
// 
//                                                                                    + ---------------------------
// ------------------------------------------------------------------------------------ Output Request Processing +

    json::Document RequestHandler::ProcessRequests(const json::Document& document) const {
        using namespace std::literals;
        std::stringstream ss;
        ss << "["s;

		bool is_first = true;
        for (const auto& bus_or_stop : document.GetRoot().AsMap().at("stat_requests"s).AsArray()) {
            const json::Dict& to_parse = bus_or_stop.AsMap();

			if (to_parse.at("type"s) == "Map"s && renderer_) {
				renderer_->HandleRenderRequests(document);
				svg::Document render_document = renderer_->CreateMap();

				ss << "{"s;
				is_first = false;
				ss << "\"request_id\" : "s << to_parse.at("id"s).AsInt() << ", "s;
				ss << "\"map\" : "s;

				std::stringstream render_ss;
				render_document.Render(render_ss);
				json::Document json_document(render_ss.str());

				json::Print(json_document, ss);
				ss << "}"s;

				continue;
			}

			if (is_first) {
				ss << "{"s;
				is_first = false;
			}
			else {
				ss << ", {"s;
			}

			ss << "\"request_id\" : "s << to_parse.at("id"s).AsInt() << ", "s;

            if (to_parse.at("type"s) == "Bus"s) {
                const domain::BusInfo to_output = database_.GetBusInfo(to_parse.at("name"s).AsString());

				if (!to_output.is_found) {
					ss << "\"error_message\" : \"not found\""s;
					ss << "}"s;
					continue;
				}

				ss << "\"curvature\" : "s << to_output.curvature << ", "s;
				ss << "\"route_length\" : "s << to_output.actual_distance << ", "s;
				ss << "\"stop_count\" : "s << to_output.stops_on_route << ", "s;
				ss << "\"unique_stop_count\" : "s << to_output.unique_stops;
				ss << "}"s;

                continue;
            }
            
            domain::StopInfo to_output = database_.GetStopInfo(to_parse.at("name"s).AsString());

			if (!to_output.is_found) {
				ss << "\"error_message\" : \"not found\""s;
				ss << "}"s;
				continue;
			}		
			ss << "\"buses\" : ["s;

			bool is_first_local = true;
			for (const auto& bus : to_output.bus_names) {
				if (is_first_local) {
					ss << "\""s << bus << "\""s;
					is_first_local = false;
					continue;
				}

				ss << ", "s << "\""s << bus << "\""s;
			}
			ss << "]"s;
			ss << "}"s;
        }
        ss << "]"s;

		return json::Load(ss);
    }
} // request_handler