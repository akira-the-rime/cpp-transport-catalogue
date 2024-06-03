#pragma once

#include "json.h"
#include "json_builder.h"
#include "map_renderer.h"
#include "router.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace json_reader {
// ------------ [Json Reader] Definition ------------
//                                                  +
//                                                  + -------------
// -------------------------------------------------- Json Reader +

	class JsonReader final {
	public:
		JsonReader(catalogue::TransportCatalogue& database,
			map_renderer::MapRenderer& renderer,
			transport_router::TransportRouter& transport_router);

		json::Document HandleRequests(const json::Document& document);

	private:
		struct CatalogueStopsFillingParameters final {
			std::unordered_map<std::string_view, std::pair<std::vector<std::string_view>, bool>>& buses;
			std::unordered_map<std::string_view, json::Dict>& stops_and_destinations;
		};

		struct MakeStopDatabaseParameters final {
			std::map<std::string_view, std::pair<std::vector<std::string_view>, bool>>& buses_and_stops;
			std::deque<domain::Stop>& deque_stops;
		};

		struct MakeBusDatabaseParameters final {
			std::map<std::string_view, std::pair<std::vector<std::string_view>, bool>>& buses_and_stops;
			std::deque<domain::Stop>& deque_stops;
			std::set<std::string_view>& sorted_stops;
			std::deque<std::pair<domain::Bus, bool>>& routes;
		};

		void FillRouter();
		void HandleBaseRequests(const json::Document& document);
		void HandleRenderRequests(const json::Document& json_document);
		void HandleRoutingSettingsRequests(const json::Document& json_document);
		json::Document HandleStatRequests(const json::Document& document) const;

		void CatalogueDestinationsFilling(std::unordered_map<std::string_view, json::Dict>& stops_and_destinations);
		void CatalogueBusesFilling(std::unordered_map<std::string_view, std::pair<std::vector<std::string_view>, bool>>& buses);

		const svg::Color ChooseColor(const json::Node& to_process) const;
		void ExtractSettings(const json::Document& document);

		void ProcessMapRequest(const json::Dict& to_parse, json::Builder& builder) const;
		void ProcessBusRequest(const json::Dict& to_parse, json::Builder& builder) const;
		void ProcessStopRequest(const json::Dict& to_parse, json::Builder& builder) const;
		void ProcessRouteRequest(const json::Dict& to_parse, json::Builder& builder) const;

		void CatalogueStopsFilling(const json::Document& document, const CatalogueStopsFillingParameters& parameters);
		void MakeStopDatabase(const json::Document& document, const MakeStopDatabaseParameters& parameters);
		void MakeBusDatabase(const MakeBusDatabaseParameters& parameters);

		catalogue::TransportCatalogue& database_;
		map_renderer::MapRenderer& renderer_;
		transport_router::TransportRouter& transport_router_;
	};
} // namespace input_reader