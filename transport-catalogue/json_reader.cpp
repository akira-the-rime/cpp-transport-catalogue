#include "json_reader.h"

namespace json_reader {
// -------------------------------- [Json Reader] Realization -------------------------
//                                                                                    +
//                                                                                    + -------------
// ------------------------------------------------------------------------------------ Constructor +

	JsonReader::JsonReader(catalogue::TransportCatalogue& database)
		: database_(database) {
	}

// 
// 
//                                                                                    + ------------
// ------------------------------------------------------------------------------------ Filling in +

	void JsonReader::FillTransportCatalogue(const json::Document& document) {
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
				stops_and_destinations[to_parse.at("name"s).AsString()] = to_parse.at("road_distances"s).AsMap();
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

} // namespace input_reader