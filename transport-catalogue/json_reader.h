#pragma once

#include <deque>
#include <map>
#include <set>
#include <utility>

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

namespace json_reader {
// --------------------------------- [Json Reader] Definition -------------------------
//                                                                                    +
//                                                                                    + -------------
// ------------------------------------------------------------------------------------ Json Reader +

	class JsonReader {
	public:
		JsonReader(catalogue::TransportCatalogue& database, map_renderer::MapRenderer& renderer);
		void HandleBaseRequests(const json::Document& document);
		void HandleRenderRequests(const json::Document& json_document);
		json::Document HandleStatRequests(const json::Document& document) const;

	private:
		void CatalogueStopsFilling(const json::Document& document,
			std::unordered_map<std::string_view, std::vector<std::string_view>>& buses,
			std::unordered_map<std::string_view, json::Dict>& stops_and_destinations);

		void CatalogueDestinationsFilling(std::unordered_map<std::string_view, json::Dict>& stops_and_destinations);
		void CatalogueBusesFilling(std::unordered_map<std::string_view, std::vector<std::string_view>>& buses);

		const svg::Color ChooseColor(const json::Node& to_process) const;
		void ExtractSettings(const json::Document& json_document);

		void MakeStopDatabase(const json::Document& json_document,
			std::map<std::string_view, std::pair<std::vector<std::string_view>, bool>>& buses_and_stops, 
			std::deque<domain::Stop>& deque_stops);

		void MakeBusDatabase(std::map<std::string_view, std::pair<std::vector<std::string_view>, bool>>& buses_and_stops,
			std::deque<domain::Stop>& deque_stops,
			std::set<std::string_view>& sorted_stops,
			std::deque<std::pair<domain::Bus, bool>>& routes);

		void ProcessMapRequest(const json::Dict& to_parse, json::Builder& builder) const;
		void ProcessBusRequest(const json::Dict& to_parse, json::Builder& builder) const;
		void ProcessStopRequest(const json::Dict& to_parse, json::Builder& builder) const;
		
		catalogue::TransportCatalogue& database_;
		map_renderer::MapRenderer& renderer_;
	};
} // namespace input_reader