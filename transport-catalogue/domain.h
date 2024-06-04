#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <string>

#include "geo.h"
#include "graph.h"
#include "router.h"

namespace domain {
//------------ [Domain Structs] Definition ------------
//                                                     +
//                                                     + --------------------
// ----------------------------------------------------- Stop & Bus structs +

	struct Stop final {
		std::string name;
		geo::Coordinates coordinates;

		bool operator==(std::string_view rhs) const;
		bool operator!=(std::string_view rhs) const;
	};

	struct Bus final {
		std::string name;
		std::vector<Stop*> stops_with_duplicates;
		bool is_roundtrip = {};

		bool operator==(std::string_view rhs) const;
		bool operator!=(std::string_view rhs) const;
	};

// 
// 
//                                                     + -------------
// ----------------------------------------------------- Auxiliaries +

	struct Compartor final {
		using is_transparent = std::false_type;
		bool operator()(const Bus* lhs, const Bus* rhs) const;
	};

	struct Hasher final {
		std::size_t operator()(const std::pair<std::string_view, std::string_view>& to_hash) const;

	private:
		std::hash<std::string_view> sw_hasher;
	};

// 
// 
//                                                     + -----------------
// ----------------------------------------------------- Stop & Bus Info +

	struct StopInfo final {
		std::string_view name;
		bool is_found = false;

		std::vector<std::string_view> bus_names;
	};

	struct BusInfo final {
		std::string_view name;
		bool is_found = false;

		std::size_t stops_on_route = 0;
		std::size_t unique_stops = 0;
		std::size_t actual_distance = 0;
		double pure_distance = 0.0;
		double curvature = 0.0;
	};

// 
// 
//                                                     + ------------
// ----------------------------------------------------- Route Data +

	struct RoutingSettings final {
		std::uint16_t bus_wait_time = {};
		double bus_velocity = {};
	};

	struct Data final {
		std::optional<typename graph::Router<double>::RouteInfo> route;
		const graph::DirectedWeightedGraph<double>& graph;
		const graph::Router<double>* router;
		std::uint16_t bus_wait_time;
		const std::deque<domain::Stop>& graph_stops;
		const std::unordered_map<std::pair<std::string_view, std::string_view>, std::map<std::size_t, std::deque<std::string_view>>, domain::Hasher>& spans;
	};

	struct SpanInfo final {
		std::size_t span_count;
		const std::deque<std::string_view>* buses = nullptr;
	};
} // namespace domain