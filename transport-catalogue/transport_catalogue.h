#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "geo.h"

namespace catalogue {
	struct Stop {
		std::string name;
		geo::Coordinates coordinates;

		bool operator==(std::string_view rhs) const {
			return std::string_view(name) == rhs;
		}

		bool operator!=(std::string_view rhs) const {
			return std::string_view(name) != rhs;
		}
	};

	struct Bus {
		std::string name;
		std::vector<Stop*> buses_with_duplicates;

		bool operator==(std::string_view rhs) const {
			return std::string_view(name) == rhs;
		}

		bool operator!=(std::string_view rhs) const {
			return std::string_view(name) != rhs;
		}
	};

	struct BusInfo {
		std::string_view name;
		bool is_found = false;

		std::size_t stops_on_route = 0;
		std::size_t unique_stops = 0;
		std::size_t actual_distance = 0;
		double pure_distance = 0.0;
		double curvature = 0.0;
	};

	struct StopInfo {
		std::string_view name;
		bool is_found = false;

		std::vector<std::string_view> bus_names;
	};

	struct Compartor {
		using is_transparent = std::false_type;
		bool operator()(const Bus* lhs, const Bus* rhs) const {
			return lhs->name < rhs->name;
		}
	};

	struct Hasher {
		std::size_t operator()(const std::pair<std::string_view, std::string_view>& to_hash) const {
			std::size_t first = sw_hasher(to_hash.first);
			std::size_t second = sw_hasher(to_hash.second);
			return first + second * 37;
		}
	private:
		std::hash<std::string_view> sw_hasher;
	};

	class TransportCatalogue {
	public:
		void AddStop(const std::string& stop, const geo::Coordinates& coordinates);
		void AddDestination(const std::string& stop, const std::unordered_map<std::string_view, std::size_t>& dst);
		void AddBus(const std::string& bus, const std::vector<std::string_view>& proper_stops);

		Stop* FindStop(std::string_view stop);
		Bus* FindBus(std::string_view bus);
		
		const BusInfo GetBusInfo(std::string_view bus) const;
		const StopInfo GetStopInfo(std::string_view stop) const;
	private:
		std::size_t ReturnAmoutOfUniqueStopsForBus(std::string_view bus) const;
		std::optional<const Bus*> FindBus(std::string_view bus) const;
		std::optional<const std::vector<Stop*>*> ReturnStopsForBus(std::string_view bus) const;
		std::optional<const std::set<Bus*, Compartor>*> ReturnBusesForStop(std::string_view stop) const;
		size_t ComputeActualLength(std::string_view bus) const;
		double ComputePureLength(std::string_view bus) const;

		std::deque<Stop> deque_stops;
		std::deque<Bus> deque_buses;

		std::unordered_map<std::string_view, std::set<Bus*, Compartor>> stops;
		std::unordered_map<std::pair<std::string_view, std::string_view>, std::size_t, Hasher> destinations;
		std::unordered_map<std::string_view, std::unordered_set<Stop*>> buses;  
	};
}