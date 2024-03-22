#include <algorithm>
#include <vector>

#include "transport_catalogue.h"

namespace catalogue {
	void TransportCatalogue::AddStop(std::string_view name, geo::Coordinates&& coordinates) {
		deque_stops.push_back({ std::string(std::move(name)), std::move(coordinates) });
		stops[deque_stops.back().name];
	}

	void TransportCatalogue::AddBus(std::string_view bus, const std::vector<std::string_view>& proper_stops) {
		deque_buses.push_back({ std::string(std::move(bus)) });
		for (const auto& stop : proper_stops) {
			stops.at(stop).insert(&deque_buses.back());

			Stop* value = FindStop(stop);
			buses[deque_buses.back().name].insert(value);
			buses_wd[deque_buses.back().name].push_back(value);
		}
	}

	Stop* TransportCatalogue::FindStop(std::string_view stop) {
		return &*std::find(deque_stops.begin(), deque_stops.end(), stop);
	}

	std::size_t TransportCatalogue::ReturnAmoutOfUniqueStopsForBus(std::string_view name) const {
		return buses.at(name).size();
	}

	std::optional<const std::deque<Stop*>*> TransportCatalogue::ReturnStopsForBus(std::string_view name) const {
		if (buses_wd.count(name)) {
			return &buses_wd.at(name);
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

	double TransportCatalogue::ComputeLength(std::string_view name) const {
		double distance = 0.0;
		geo::Coordinates first, second;

		bool is_first = true;
		for (const auto& stop : *ReturnStopsForBus(name).value()) {
			first = stop->coordinates;
			if (!is_first) {
				distance += geo::ComputeDistance(first, second);
			}
			second = first;
			is_first = false;
		}
		return distance;
	}
}