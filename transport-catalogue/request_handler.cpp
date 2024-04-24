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
//                                                                                    + ------------------------
// ------------------------------------------------------------------------------------ Map Request Processing +

	json::Node RequestHandler::ProcessMapRequest(const json::Dict& to_parse, const json::Document& document) const {
		using namespace std::literals;

		renderer_->HandleRenderRequests(document);
		svg::Document render_document = renderer_->RenderMap();

		std::stringstream ss;
		render_document.Render(ss);

		json::Dict to_make;
		to_make.emplace("map"s, ss.str());
		to_make.emplace("request_id"s, to_parse.at("id"s).AsInt());

		return { to_make };
	}

// 
// 
//                                                                                    + -------------------------
// ------------------------------------------------------------------------------------ Bus Request Processing +

	json::Node RequestHandler::ProcessBusRequest(const json::Dict& to_parse) const {
		using namespace std::literals;

		json::Dict to_make;
		const domain::BusInfo to_output = database_.GetBusInfo(to_parse.at("name"s).AsString());

		if (!to_output.is_found) {
			to_make.emplace("error_message"s, "not found"s);
			to_make.emplace("request_id"s, to_parse.at("id"s).AsInt());
			return { to_make };
		}

		to_make.emplace("curvature"s, to_output.curvature);
		to_make.emplace("route_length"s, static_cast<int>(to_output.actual_distance));
		to_make.emplace("stop_count"s, static_cast<int>(to_output.stops_on_route));
		to_make.emplace("unique_stop_count"s, static_cast<int>(to_output.unique_stops));

		to_make.emplace("request_id"s, to_parse.at("id"s).AsInt());

		return { to_make };
	}

// 
// 
//                                                                                    + ------------------------
// ------------------------------------------------------------------------------------ Stop Request Processing +

	json::Node RequestHandler::ProcessStopRequest(const json::Dict& to_parse) const {
		using namespace std::literals;

		json::Dict to_make;
		const domain::StopInfo to_output = database_.GetStopInfo(to_parse.at("name"s).AsString());

		if (!to_output.is_found) {
			to_make.emplace("error_message"s, "not found"s);
			to_make.emplace("request_id"s, to_parse.at("id"s).AsInt());
			return { to_make };
		}

		json::Array buses;
		for (const auto& bus : to_output.bus_names) {
			buses.push_back(std::string(bus));
		}

		to_make.emplace("buses"s, buses);
		to_make.emplace("request_id"s, to_parse.at("id"s).AsInt());

		return { to_make };
	}

// 
// 
//                                                                                    + -----------------------
// ------------------------------------------------------------------------------------ Output Request Facade +

    json::Document RequestHandler::ProcessRequests(const json::Document& document) const {
        using namespace std::literals;

		json::Array node_argument;

        for (const auto& bus_or_stop : document.GetRoot().AsMap().at("stat_requests"s).AsArray()) {
            const json::Dict& to_parse = bus_or_stop.AsMap();

			if (to_parse.at("type"s) == "Map"s && renderer_) {
				node_argument.push_back(ProcessMapRequest(to_parse, document));
				continue;
			}

            if (to_parse.at("type"s) == "Bus"s) {
				node_argument.push_back(ProcessBusRequest(to_parse));
                continue;
            }
            
			node_argument.push_back(ProcessStopRequest(to_parse));
        }

		return json::Document(json::Node{ node_argument });
    }
} // request_handler