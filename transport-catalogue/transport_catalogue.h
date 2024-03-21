#pragma once

#include <cstddef>
#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "geo.h"

namespace catalogue {
	struct Stop {
		std::string name_;
		geo::Coordinates coordinates;

		bool operator==(std::string_view rhs) const {
			return std::string_view(name_) == rhs;
		}

		bool operator!=(std::string_view rhs) const {
			return std::string_view(name_) != rhs;
		}
	};

	struct Bus {
		std::string name_;
		double length = 0.0;

		bool operator==(std::string_view rhs) const {
			return std::string_view(name_) == rhs;
		}

		bool operator!=(std::string_view rhs) const {
			return std::string_view(name_) != rhs;
		}
	};

	struct Compartor {
		using is_transparent = std::false_type;

		bool operator()(const Bus* lhs, const Bus* rhs) const {
			return lhs->name_ < rhs->name_;
		}
	};

	class TransportCatalogue {
	public:
		using stop_list_with_buses = std::unordered_map<std::string_view, std::set<Bus*, Compartor>>;                             // stops
		using bus_list_with_stops_no_duplicates = std::unordered_map<std::string_view, std::unordered_map<Stop*, std::size_t>>;    // buses
		using bus_list_with_stops_with_duplicates = std::unordered_map<std::string_view, std::deque<Stop*>>;                      // buses_wd

		void AddStop(std::string_view name, geo::Coordinates&& coordinates);
		void AddBus(std::string_view bus, bool& is_first, std::string_view stop);
		const std::deque<Stop*>& ReturnStopsForBusWithDuplicates(std::string_view name) const;
		std::optional<const std::unordered_map<Stop*, std::size_t>*> ReturnStopsForBus(std::string_view name) const;
		std::optional<const std::set<Bus*, Compartor>*> ReturnBusesForStop(std::string_view name) const;
	private:
		Stop& FindStop(std::string_view stop);
		std::deque<Stop> deque_stops;
		std::deque<Bus> deque_buses;

		stop_list_with_buses stops;
		bus_list_with_stops_no_duplicates buses;
		bus_list_with_stops_with_duplicates buses_wd;
	};
}