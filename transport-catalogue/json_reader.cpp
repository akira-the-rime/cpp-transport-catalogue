#include <sstream>
#include <thread>

#include "json_reader.h"

namespace json_reader {
// ------------ [Json Reader] Realization ------------
//                                                   +
//                                                   + -------------
// --------------------------------------------------- Constructor +

	JsonReader::JsonReader(catalogue::TransportCatalogue& database,
		map_renderer::MapRenderer& renderer,
		transport_router::TransportRouter& transport_router)

		: database_(database)
		, renderer_(renderer)
		, transport_router_(transport_router) {
	}

// 
// 
//                                                   + -------------
// --------------------------------------------------- Auxiliaries +

	const svg::Color JsonReader::ChooseColor(const json::Node& to_process) const {
		if (to_process.IsString()) {
			return to_process.AsString();
		}
		else if (to_process.AsArray().size() == 3) {
			svg::Rgb to_add = { static_cast<std::uint8_t>(to_process.AsArray().front().AsInt()),
				static_cast<std::uint8_t>(to_process.AsArray().at(1).AsInt()),
				static_cast<std::uint8_t>(to_process.AsArray().back().AsInt()) };
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
//                                                   + ---------------------------
// --------------------------------------------------- Catalogue [Stops filling] +

	void JsonReader::CatalogueStopsFilling(const json::Document& document, const JsonReader::CatalogueStopsFillingParameters& parameters) {
		using namespace std::literals;

		for (const auto& bus_or_stop : document.GetRoot().AsMap().at("base_requests"s).AsArray()) {
			const json::Dict& to_parse = bus_or_stop.AsMap();

			if (to_parse.at("type"s) == "Bus"s) {
				std::vector<std::string_view> proper_stops;

				bool is_roundtrop = {};
				if (to_parse.at("is_roundtrip"s).AsBool()) {
					for (const auto& stop : to_parse.at("stops"s).AsArray()) {
						proper_stops.push_back(stop.AsString());
					}

					is_roundtrop = true;
				}
				else {
					for (const auto& stop : to_parse.at("stops"s).AsArray()) {
						proper_stops.push_back(stop.AsString());
					}

					for (auto it = to_parse.at("stops"s).AsArray().rbegin() + 1; it != to_parse.at("stops"s).AsArray().rend(); ++it) {
						const std::string& stop = it->AsString();
						proper_stops.push_back(stop);
					}

					is_roundtrop = false;
				}

				parameters.buses[to_parse.at("name"s).AsString()] = { proper_stops, is_roundtrop };
				continue;
			}

			database_.AddStop(to_parse.at("name"s).AsString(), { to_parse.at("latitude"s).AsDouble(), to_parse.at("longitude"s).AsDouble() });

			if (to_parse.contains("road_distances"s)) {
				parameters.stops_and_destinations[to_parse.at("name"s).AsString()] = to_parse.at("road_distances"s).AsMap();
			}
		}
	}

// 
// 
//                                                   + ----------------------------------
// --------------------------------------------------- Catalogue [Destinations filling] +

	void JsonReader::CatalogueDestinationsFilling(std::unordered_map<std::string_view, json::Dict>& stops_and_destinations) {
		for (const auto& [stop, destinations] : stops_and_destinations) {
			for (const auto& [destination, length] : destinations) {
				database_.AddDestination(std::string(stop), destination, length.AsInt());
			}
		}
	}

// 
// 
//                                                   + ---------------------------
// --------------------------------------------------- Catalogue [Buses filling] +

	void JsonReader::CatalogueBusesFilling(std::unordered_map<std::string_view, std::pair<std::vector<std::string_view>, bool>>& buses) {
		for (const auto& [name, proper_stops] : buses) {
			database_.AddBus(std::string(name), proper_stops.first, proper_stops.second);
		}
	}

//
// 
//                                                   + ----------------
// --------------------------------------------------- Router filling +

	void JsonReader::FillRouter() {
		const std::deque<domain::Stop>* stops = database_.GetAllStops();
		std::thread set_stops_thread([&]() {
			transport_router_.SetStops(stops->begin(), stops->end());
		});

		const std::deque<domain::Bus>* buses = database_.GetAllBuses();
		std::thread set_buses_thread([&]() {
			transport_router_.SetBuses(buses->begin(), buses->end());
		});
		
		const std::unordered_map<std::pair<std::string_view, std::string_view>, std::size_t, domain::Hasher>* destinations = database_.GetDestinations();
		std::thread set_destinatios_thread([&]() {
			transport_router_.SetDesinations(destinations->begin(), destinations->end());
		});

		set_stops_thread.join();
		set_buses_thread.join();
		set_destinatios_thread.join();

		transport_router_.FillRouter();
	}

//
// 
//                                                   + ----------------
// --------------------------------------------------- Filling Facade +

	void JsonReader::HandleBaseRequests(const json::Document& document) {
		using namespace std::literals;

		std::unordered_map<std::string_view, json::Dict> stops_and_destinations;
		std::unordered_map<std::string_view, std::pair<std::vector<std::string_view>, bool>> buses;

		CatalogueStopsFilling(document, JsonReader::CatalogueStopsFillingParameters {
			.buses = buses,
			.stops_and_destinations = stops_and_destinations 
		});

		CatalogueDestinationsFilling(stops_and_destinations);
		CatalogueBusesFilling(buses);

		FillRouter();
	}

// 
// 
//                                                   + -------------------------------------
// --------------------------------------------------- Renderer filling [Settings parsing] +

	void JsonReader::ExtractSettings(const json::Document& document) {
		using namespace std::literals;

		const json::Dict& as_map = document.GetRoot().AsMap().at("render_settings"s).AsMap();

		map_renderer::Settings settings_to_be_added;

		settings_to_be_added.width = as_map.at("width"s).AsDouble();
		settings_to_be_added.height = as_map.at("height"s).AsDouble();
		settings_to_be_added.padding = as_map.at("padding"s).AsDouble();
		settings_to_be_added.line_width = as_map.at("line_width"s).AsDouble();
		settings_to_be_added.stop_radius = as_map.at("stop_radius"s).AsDouble();
		settings_to_be_added.bus_label_font_size = as_map.at("bus_label_font_size"s).AsInt();
		settings_to_be_added.bus_label_offset = { as_map.at("bus_label_offset"s).AsArray().front().AsDouble(), as_map.at("bus_label_offset"s).AsArray().back().AsDouble() };
		settings_to_be_added.stop_label_font_size = as_map.at("stop_label_font_size"s).AsInt();
		settings_to_be_added.stop_label_offset = { as_map.at("stop_label_offset"s).AsArray().front().AsDouble(), as_map.at("stop_label_offset"s).AsArray().back().AsDouble() };
		settings_to_be_added.underlayer_color = ChooseColor(as_map.at("underlayer_color"s));
		settings_to_be_added.underlayer_width = as_map.at("underlayer_width"s).AsDouble();

		renderer_.SetSettings(std::move(settings_to_be_added));
		std::vector<svg::Color> color_palette;
		for (const json::Node& color : document.GetRoot().AsMap().at("render_settings"s).AsMap().at("color_palette"s).AsArray()) {
			color_palette.push_back(ChooseColor(color));
		}

		renderer_.SetColorPalette(std::move(color_palette));
	}

// 
// 
//                                                   + -----------------------------------------
// --------------------------------------------------- Renderer filling [Making stop database] +

	void JsonReader::MakeStopDatabase(const json::Document& document, const JsonReader::MakeStopDatabaseParameters& parameters) {
		using namespace std::literals;

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

				parameters.buses_and_stops[to_parse.at("name"s).AsString()] = { proper_stops, to_parse.at("is_roundtrip"s).AsBool() };
				continue;
			}

			parameters.deque_stops.push_back({ to_parse.at("name"s).AsString(), { to_parse.at("latitude"s).AsDouble(), to_parse.at("longitude"s).AsDouble() } });
		}
	}

// 
// 
//                         + -----------------------------------------------------------
// ------------------------- Renderer filling [Making bus database & Sorted stop list] +

	void JsonReader::MakeBusDatabase(const JsonReader::MakeBusDatabaseParameters& parameters) {
		for (const auto& [bus_name, proper_stops] : parameters.buses_and_stops) {
			domain::Bus bus_to_process({ std::string(bus_name), std::vector<domain::Stop*>{} });

			for (const auto& proper_stop : proper_stops.first) {
				bus_to_process.stops_with_duplicates.push_back(&*std::find(parameters.deque_stops.begin(), parameters.deque_stops.end(), proper_stop));
				parameters.sorted_stops.insert(proper_stop);
			}

			parameters.routes.push_back({ bus_to_process, proper_stops.second });
		}
	}

// 
// 
//                                                   + -------------------------
// --------------------------------------------------- Renderer filling Facade +

	void JsonReader::HandleRenderRequests(const json::Document& document) {
		std::map<std::string_view, std::pair<std::vector<std::string_view>, bool>> buses_and_stops;

		std::deque<domain::Stop> deque_stops;
		std::set<std::string_view> sorted_stops;
		std::deque<std::pair<domain::Bus, bool>> routes;

		ExtractSettings(document);
		MakeStopDatabase(document, JsonReader::MakeStopDatabaseParameters {
			.buses_and_stops = buses_and_stops,
			.deque_stops = deque_stops 
		});

		MakeBusDatabase(JsonReader::MakeBusDatabaseParameters {
			.buses_and_stops = buses_and_stops,
			.deque_stops = deque_stops,
			.sorted_stops = sorted_stops,
			.routes = routes
		});

		renderer_.SetDequeStops(std::move(deque_stops));
		renderer_.SetSortedStops(std::move(sorted_stops));
		renderer_.SetRoutes(std::move(routes));
	}

// 
// 
//                                                   + --------------------------------------
// --------------------------------------------------- Routing Settings Requests processing +

	void JsonReader::HandleRoutingSettingsRequests(const json::Document& document) {
		using namespace std::literals;

		const json::Dict& to_parse = document.GetRoot().AsMap().at("routing_settings"s).AsMap();
		transport_router_.SetRoutingSettings(domain::RoutingSettings {
			.bus_wait_time = static_cast<std::uint16_t>(to_parse.at("bus_wait_time"s).AsInt()),
			.bus_velocity = to_parse.at("bus_velocity"s).AsDouble()
		});
	}

// 
// 
//                                                   + -------------------------------
// --------------------------------------------------- Stat Request [Map processing] +

	void JsonReader::ProcessMapRequest(const json::Dict& to_parse, json::Builder& builder) const {
		using namespace std::literals;

		svg::Document render_document = renderer_.RenderMap();

		std::stringstream ss;
		render_document.Render(ss);

		builder.StartDict().Key("map"s).Value(ss.str())
			.Key("request_id"s).Value(to_parse.at("id"s).AsInt())
			.EndDict();
	}

// 
// 
//                                                   + -------------------------------
// --------------------------------------------------- Stat Request [Bus processing] +

	void JsonReader::ProcessBusRequest(const json::Dict& to_parse, json::Builder& builder) const {
		using namespace std::literals;

		const domain::BusInfo to_output = database_.GetBusInfo(to_parse.at("name"s).AsString());

		if (!to_output.is_found) {
			builder.StartDict().Key("error_message"s).Value("not found"s)
				.Key("request_id"s).Value(to_parse.at("id"s).AsInt())
				.EndDict();
			return;
		}

		builder.StartDict().Key("curvature"s).Value(to_output.curvature)
			.Key("route_length"s).Value(static_cast<int>(to_output.actual_distance))
			.Key("stop_count"s).Value(static_cast<int>(to_output.stops_on_route))
			.Key("unique_stop_count"s).Value(static_cast<int>(to_output.unique_stops))
			.Key("request_id"s).Value(to_parse.at("id"s).AsInt())
			.EndDict();
	}

// 
// 
//                                                   + --------------------------------
// --------------------------------------------------- Stat Request [Stop processing] +

	void JsonReader::ProcessStopRequest(const json::Dict& to_parse, json::Builder& builder) const {
		using namespace std::literals;

		json::Dict to_make;
		const domain::StopInfo to_output = database_.GetStopInfo(to_parse.at("name"s).AsString());

		if (!to_output.is_found) {
			builder.StartDict().Key("error_message"s).Value("not found"s)
				.Key("request_id"s).Value(to_parse.at("id"s).AsInt())
				.EndDict();
			return;
		}

		json::Array buses;
		for (const auto& bus : to_output.bus_names) {
			buses.push_back(std::string(bus));
		}

		builder.StartDict().Key("buses"s).Value(buses)
			.Key("request_id"s).Value(to_parse.at("id"s).AsInt())
			.EndDict();
	}

// 
// 
//                                                   + ---------------------------------
// --------------------------------------------------- Stat Request [Route processing] +

	void JsonReader::ProcessRouteRequest(const json::Dict& to_parse, json::Builder& builder) const {
		using namespace std::literals;

		const std::size_t start = transport_router_.GetStopIndex(to_parse.at("from"s).AsString());
		const std::size_t end = transport_router_.GetStopIndex(to_parse.at("to"s).AsString());

		const auto& graph = transport_router_.GetGraph();
		auto route = transport_router_.GetRouter()->BuildRoute(start, end);

		if (route.has_value()) {
			std::uint16_t bus_wait_time = transport_router_.GetRoutingSettings().bus_wait_time;
			std::optional<graph::VertexId> old_to;

			builder.StartDict().Key("items"s).StartArray();

			if (route.value().edges.size() == 1) {
				builder.EndArray();

				builder.Key("request_id"s).Value(to_parse.at("id"s).AsInt())
					.Key("total_time"s).Value(route.value().weight)
					.EndDict();
			}

			for (const auto& edge : route.value().edges) {
				graph::Edge<double> vertices = graph.GetEdge(edge);

				if (!old_to.has_value() || (vertices.from + 1 == vertices.to && old_to.value() != vertices.from)) {
					builder.StartDict().Key("stop_name"s).Value(transport_router_.GetStopNameByIndex(vertices.from))
						.Key("time"s).Value(bus_wait_time)
						.Key("type"s).Value("Wait"s)
						.EndDict();

					old_to = vertices.to;
				}
				else {
					const std::string& from_name = transport_router_.GetStopNameByIndex(vertices.from);
					const std::string& to_name = transport_router_.GetStopNameByIndex(vertices.to);
					const domain::SpanInfo span_info = transport_router_.GetShortestSpan(from_name, to_name);

					builder.StartDict().Key("bus"s).Value(std::string((*span_info.buses)[0]))
						.Key("span_count"s).Value(static_cast<int>(span_info.span_count))
						.Key("time"s).Value(transport_router_.GetRouter()->BuildRoute(vertices.from, vertices.to).value().weight)
						.Key("type"s).Value("Bus"s)
						.EndDict();
				}
			}

			builder.EndArray();

			builder.Key("request_id"s).Value(to_parse.at("id"s).AsInt())
				.Key("total_time"s).Value(route.value().weight)
				.EndDict();
		}
		else {
			builder.StartDict().Key("error_message"s).Value("not found"s)
				.Key("request_id"s).Value(to_parse.at("id"s).AsInt())
				.EndDict();
		}
	}

// 
// 
//                                                   + ---------------------
// --------------------------------------------------- Stat Request Facade +

	json::Document JsonReader::HandleStatRequests(const json::Document& document) const {
		using namespace std::literals;
		json::Builder builder;
		builder.StartArray();

		for (const auto& bus_or_stop : document.GetRoot().AsMap().at("stat_requests"s).AsArray()) {
			const json::Dict& to_parse = bus_or_stop.AsMap();

			if (to_parse.at("type"s) == "Map"s) {
				ProcessMapRequest(to_parse, builder);
				continue;
			}

			if (to_parse.at("type"s) == "Bus"s) {
				ProcessBusRequest(to_parse, builder);
				continue;
			}

			if (to_parse.at("type"s) == "Stop"s) {
				ProcessStopRequest(to_parse, builder);
				continue;
			}

			ProcessRouteRequest(to_parse, builder);
		}

		return json::Document(builder.EndArray().Build());
	}

// 
// 
//                                                   + ------------------------
// --------------------------------------------------- Facade of All Requests +

	json::Document JsonReader::HandleRequests(const json::Document& document) {
		std::thread routing_settings_thread(&JsonReader::HandleRoutingSettingsRequests, &*this, std::cref(document));
		std::thread base_requests_thread(&JsonReader::HandleBaseRequests, &*this, std::cref(document));
		std::thread render_requests_thread(&JsonReader::HandleRenderRequests, &*this, std::cref(document));

		routing_settings_thread.join();
		base_requests_thread.join();
		render_requests_thread.join();

		return HandleStatRequests(document);
	}
} // namespace input_reader