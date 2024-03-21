#include <algorithm>

#include "transport_catalogue.h"

namespace catalogue {
	void TransportCatalogue::AddStop(std::string_view name, geo::Coordinates&& coordinates) {
		deque_stops.push_back({ std::string(std::move(name)), std::move(coordinates) });
		stops[deque_stops.back().name_];
	}

	void TransportCatalogue::AddBus(std::string_view bus, bool& is_first, std::string_view stop) {
		if (is_first) {
			deque_buses.push_back({ std::string(std::move(bus)) });
			is_first = false;
		}
		stops.at(stop).insert(&deque_buses.back());

		Stop* value = &FindStop(stop);
		if (!buses[deque_buses.back().name_].count(value)) {
			buses[deque_buses.back().name_][value] = 1;
		}
		else {
			++buses[deque_buses.back().name_][value];
		}

		buses_wd[deque_buses.back().name_].push_back(value);
	}

	Stop& TransportCatalogue::FindStop(std::string_view stop) {
		return *std::find(deque_stops.begin(), deque_stops.end(), stop);
	}

	const std::deque<Stop*>& TransportCatalogue::ReturnStopsForBusWithDuplicates(std::string_view name) const {
		return buses_wd.at(name);
	}

	std::optional<const std::unordered_map<Stop*, std::size_t>*> TransportCatalogue::ReturnStopsForBus(std::string_view name) const {
		if (buses.count(name)) {
			return &buses.at(name);
		}
		else {
			return std::nullopt;
		}
	}

	std::optional<const std::set<Bus*, Compartor>*> TransportCatalogue::ReturnBusesForStop(std::string_view name) const {
		if (stops.count(name)) {
			return &stops.at(name);
		}
		else {
			return std::nullopt;
		}
	}
}