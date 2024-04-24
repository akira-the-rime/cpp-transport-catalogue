#include <sstream>

#include "json_reader.h"

namespace json_reader {
// -------------------------------- [Json Reader] Realization -------------------------
//                                                                                    +
//                                                                                    + -------------
// ------------------------------------------------------------------------------------ Constructor +

	JsonReader::JsonReader(catalogue::TransportCatalogue& database, map_renderer::MapRenderer& renderer)
		: database_(database) 
		, renderer_(renderer) {
	}

// 
// 
//                                                                                    + -------------
// ------------------------------------------------------------------------------------ Auxiliaries +

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
//                                                                                    + -------------------------
// ------------------------------------------------------------------------------------ Catalogue Stops filling +

	void JsonReader::CatalogueStopsFilling(const json::Document& document,
		std::unordered_map<std::string_view, std::vector<std::string_view>>& buses,
		std::unordered_map<std::string_view, json::Dict>& stops_and_destinations) {

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

				buses[to_parse.at("name"s).AsString()] = proper_stops;
				continue;
			}

			database_.AddStop(to_parse.at("name"s).AsString(), { to_parse.at("latitude"s).AsDouble(), to_parse.at("longitude"s).AsDouble() });

			if (to_parse.contains("road_distances"s)) {
				stops_and_destinations[to_parse.at("name"s).AsString()] = to_parse.at("road_distances"s).AsMap();
			}
		}
	}

// 
// 
//                                                                                    + --------------------------------
// ------------------------------------------------------------------------------------ Catalogue Destinations filling +

	void JsonReader::CatalogueDestinationsFilling(std::unordered_map<std::string_view, json::Dict>& stops_and_destinations) {
		for (const auto& [stop, destinations] : stops_and_destinations) {
			for (const auto& [destination, length] : destinations) {
				database_.AddDestination(std::string(stop), destination, length.AsInt());
			}
		}
	}

// 
// 
//                                                                                    + -------------------------
// ------------------------------------------------------------------------------------ Catalogue Buses filling +

	void JsonReader::CatalogueBusesFilling(std::unordered_map<std::string_view, std::vector<std::string_view>>& buses) {
		for (const auto& [name, proper_stops] : buses) {
			database_.AddBus(std::string(name), proper_stops);
		}
	}

//
// 
//                                                                                    + --------------------------
// ------------------------------------------------------------------------------------ Catalogue filling Facade +

	void JsonReader::HandleBaseRequests(const json::Document& document) {
		using namespace std::literals;

		std::unordered_map<std::string_view, json::Dict> stops_and_destinations;
		std::unordered_map<std::string_view, std::vector<std::string_view>> buses;

		CatalogueStopsFilling(document, buses, stops_and_destinations);
		CatalogueDestinationsFilling(stops_and_destinations);
		CatalogueBusesFilling(buses);
	}

// 
// 
//                                                                                    + -------------------------------------
// ------------------------------------------------------------------------------------ Renderer filling [Settings parsing] +

	void JsonReader::ExtractSettings(const json::Document& json_document) {
		using namespace std::literals;

		const json::Dict& as_map = json_document.GetRoot().AsMap().at("render_settings"s).AsMap();

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
		for (const json::Node& color : json_document.GetRoot().AsMap().at("render_settings"s).AsMap().at("color_palette"s).AsArray()) {
			color_palette.push_back(ChooseColor(color));
		}

		renderer_.SetColorPalette(std::move(color_palette));
	}

	// 
	// 
	//                                                                                    + -----------------------------------------
	// ------------------------------------------------------------------------------------ Renderer filling [Making stop database] +

	void JsonReader::MakeStopDatabase(const json::Document& json_document,
		std::map<std::string_view, std::pair<std::vector<std::string_view>, bool>>& buses_and_stops,
		std::deque<domain::Stop>& deque_stops) {

		using namespace std::literals;

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

			deque_stops.push_back({ to_parse.at("name"s).AsString(), { to_parse.at("latitude"s).AsDouble(), to_parse.at("longitude"s).AsDouble() } });
		}
	}

	// 
	// 
	//                                                                                    + -----------------------------------------------------------
	// ------------------------------------------------------------------------------------ Renderer filling [Making bus database & Sorted stop list] +

	void JsonReader::MakeBusDatabase(std::map<std::string_view, std::pair<std::vector<std::string_view>, bool>>& buses_and_stops,
		std::deque<domain::Stop>& deque_stops,
		std::set<std::string_view>& sorted_stops,
		std::deque<std::pair<domain::Bus, bool>>& routes) {

		for (const auto& [bus_name, proper_stops] : buses_and_stops) {
			domain::Bus bus_to_process({ std::string(bus_name), std::vector<domain::Stop*>{} });

			for (const auto& proper_stop : proper_stops.first) {
				bus_to_process.stops_with_duplicates.push_back(&*std::find(deque_stops.begin(), deque_stops.end(), proper_stop));
				sorted_stops.insert(proper_stop);
			}

			routes.push_back({ bus_to_process, proper_stops.second });
		}
	}

	// 
	// 
	//                                                                                    + -------------------------
	// ------------------------------------------------------------------------------------ Renderer filling Facade +

	void JsonReader::HandleRenderRequests(const json::Document& json_document) {
		std::map<std::string_view, std::pair<std::vector<std::string_view>, bool>> buses_and_stops;

		std::deque<domain::Stop> deque_stops;
		std::set<std::string_view> sorted_stops;
		std::deque<std::pair<domain::Bus, bool>> routes;

		ExtractSettings(json_document);
		MakeStopDatabase(json_document, buses_and_stops, deque_stops);
		MakeBusDatabase(buses_and_stops, deque_stops, sorted_stops, routes);

		renderer_.SetDequeStops(std::move(deque_stops));
		renderer_.SetSortedStops(std::move(sorted_stops));
		renderer_.SetRoutes(std::move(routes));
	}

// 
// 
//                                                                                    + -----------------------------
// ------------------------------------------------------------------------------------ Stat Request Map processing +

	json::Node JsonReader::ProcessMapRequest(const json::Dict& to_parse) const {
		using namespace std::literals;

		svg::Document render_document = renderer_.RenderMap();

		std::stringstream ss;
		render_document.Render(ss);

		json::Dict to_make;
		to_make.emplace("map"s, ss.str());
		to_make.emplace("request_id"s, to_parse.at("id"s).AsInt());

		return { to_make };
	}

// 
// 
//                                                                                    + -----------------------------
// ------------------------------------------------------------------------------------ Stat Request Bus processing +

	json::Node JsonReader::ProcessBusRequest(const json::Dict& to_parse) const {
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
//                                                                                    + ------------------------------
// ------------------------------------------------------------------------------------ Stat Request Stop processing +

	json::Node JsonReader::ProcessStopRequest(const json::Dict& to_parse) const {
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
//                                                                                    + ---------------------
// ------------------------------------------------------------------------------------ Stat Request Facade +

	json::Document JsonReader::HandleStatRequests(const json::Document& document) const {
		using namespace std::literals;

		json::Array node_argument;

		for (const auto& bus_or_stop : document.GetRoot().AsMap().at("stat_requests"s).AsArray()) {
			const json::Dict& to_parse = bus_or_stop.AsMap();

			if (to_parse.at("type"s) == "Map"s) {
				node_argument.push_back(ProcessMapRequest(to_parse));
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
} // namespace input_reader