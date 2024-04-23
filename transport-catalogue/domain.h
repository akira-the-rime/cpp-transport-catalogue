#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "geo.h"

namespace domain {
// ------------------------------- [Domain Structs] Definition ------------------------
//                                                                                    +
//                                                                                    + --------------------
// ------------------------------------------------------------------------------------ Bus & Stop structs +
	struct Stop final {
		std::string name;
		geo::Coordinates coordinates;

		bool operator==(std::string_view rhs) const;
		bool operator!=(std::string_view rhs) const;
	};

	struct Bus final {
		std::string name;
		std::vector<Stop*> stops_with_duplicates;

		bool operator==(std::string_view rhs) const;
		bool operator!=(std::string_view rhs) const;
	};

//
// 
//                                                                                    + --------------------
// ------------------------------------------------------------------------------------ Bus' & Stop's Info +
	struct BusInfo final {
		std::string_view name;
		bool is_found = false;

		std::size_t stops_on_route = 0;
		std::size_t unique_stops = 0;
		std::size_t actual_distance = 0;
		double pure_distance = 0.0;
		double curvature = 0.0;
	};

	struct StopInfo final {
		std::string_view name;
		bool is_found = false;

		std::vector<std::string_view> bus_names;
	};

// 
// 
//                                                                                    + -------
// ------------------------------------------------------------------------------------ Other +
	struct Compartor final {
		using is_transparent = std::false_type;
		bool operator()(const Bus* lhs, const Bus* rhs) const;
	};

	struct Hasher final {
		std::size_t operator()(const std::pair<std::string_view, std::string_view>& to_hash) const;

	private:
		std::hash<std::string_view> sw_hasher;
	};
} // namespace domain