#pragma once

#include <cstddef>
#include <deque>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
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
		std::deque<Stop*> buses_wd;

		bool operator==(std::string_view rhs) const {
			return std::string_view(name) == rhs;
		}

		bool operator!=(std::string_view rhs) const {
			return std::string_view(name) != rhs;
		}
	};

	struct Compartor {
		using is_transparent = std::false_type;
		bool operator()(const Bus* lhs, const Bus* rhs) const {
			return lhs->name < rhs->name;
		}
	};

	class TransportCatalogue {
	public:
		void AddStop(const std::string& stop, const geo::Coordinates& coordinates);
		void AddBus(const std::string& bus, const std::vector<std::string_view>& proper_stops);

		Stop* FindStop(std::string_view stop);
		Bus* FindBus(std::string_view bus);
		
		std::stringstream GetBusInfo(std::string_view bus) const;
		std::stringstream GetStopInfo(std::string_view stop) const;
	private:
		std::size_t ReturnAmoutOfUniqueStopsForBus(std::string_view bus) const;
		std::optional<const Bus*> FindBus(std::string_view bus) const;
		std::optional<const std::deque<Stop*>*> ReturnStopsForBus(std::string_view bus) const;
		std::optional<const std::set<Bus*, Compartor>*> ReturnBusesForStop(std::string_view stop) const;
		double ComputeLength(std::string_view bus) const;

		std::deque<Stop> deque_stops;
		std::deque<Bus> deque_buses;

		std::unordered_map<std::string_view, std::set<Bus*, Compartor>> stops;
		std::unordered_map<std::string_view, std::unordered_set<Stop*>> buses;  
	};
}