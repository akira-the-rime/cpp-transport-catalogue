#include <algorithm>

#include "transport_catalogue.h"

namespace catalogue {
	void TransportCatalogue::AddStop(const std::string& stop, const geo::Coordinates& coordinates) {
		deque_stops.emplace_back(stop, coordinates, std::unordered_map<std::string_view, std::size_t>{});
		stops[deque_stops.back().name];
	}

	void TransportCatalogue::AddDestination(const std::string& stop,
		const std::unordered_map<std::string_view, std::size_t>& dst) {

		Stop* stop_to_process = FindStop(stop);
		for (const auto& [destination, length] : dst) {
			Stop* retrieved_stop = FindStop(destination);
			stop_to_process->destinations[retrieved_stop->name] = length;

			if (!retrieved_stop->destinations.contains(stop_to_process->name)) {
				retrieved_stop->destinations[stop_to_process->name] = length;
			}
		}
	}

	void TransportCatalogue::AddBus(const std::string& bus, const std::vector<std::string_view>& proper_stops) {
		deque_buses.emplace_back(bus, std::deque<Stop*>{});

		for (const auto& stop : proper_stops) {
			Bus* bus_to_process = &deque_buses.back();
			stops.at(stop).insert(bus_to_process);

			Stop* stop_to_process = FindStop(stop);
			buses[bus_to_process->name].insert(stop_to_process);

			FindBus(bus)->buses_wd.push_back(stop_to_process);
		}
	}

	Stop* TransportCatalogue::FindStop(std::string_view stop) {
		return &*std::find(deque_stops.begin(), deque_stops.end(), stop);
	}

	Bus* TransportCatalogue::FindBus(std::string_view bus) {
		return &*std::find(deque_buses.begin(), deque_buses.end(), bus);
	}

	const BusInfo TransportCatalogue::GetBusInfo(std::string_view bus) const {
		using namespace std::literals;

		BusInfo to_output;
		std::optional<const std::deque<catalogue::Stop*>*> to_deem = ReturnStopsForBus(bus);

		if (!to_deem.has_value()) {
			to_output.name = bus;
			return to_output;
		}

		to_output.name = bus;
		to_output.stops_on_route = to_deem.value()->size();
		to_output.unique_stops = ReturnAmoutOfUniqueStopsForBus(bus);

		to_output.actual_distance = ComputeActualLength(bus);
		to_output.pure_distance = ComputePureLength(bus);
		to_output.curvature = to_output.actual_distance / to_output.pure_distance;

		to_output.is_found = true;
		return to_output;
	}

	const StopInfo TransportCatalogue::GetStopInfo(std::string_view stop) const {
		using namespace std::literals;

		StopInfo to_output;
		const std::optional<const std::set<catalogue::Bus*, catalogue::Compartor>*> to_deem = ReturnBusesForStop(stop);

		if (!to_deem.has_value()) {
			to_output.name = stop;
			return to_output;
		}
		else if (!to_deem.value()->size()) {
			to_output.name = stop;
			to_output.is_found = true;
			return to_output;
		}

		to_output.name = stop;
		for (const auto& bus : *to_deem.value()) {
			to_output.bus_names.push_back(bus->name);
		}
		to_output.is_found = true;
		return to_output;
	}

	std::size_t TransportCatalogue::ReturnAmoutOfUniqueStopsForBus(std::string_view bus) const {
		return buses.at(bus).size();
	}

	std::optional<const Bus*> TransportCatalogue::FindBus(std::string_view bus) const {
		auto it = std::find(deque_buses.begin(), deque_buses.end(), bus);
		if (it != deque_buses.end()) {
			return &*it;
		}
		return std::nullopt;
	}

	std::optional<const std::deque<Stop*>*> TransportCatalogue::ReturnStopsForBus(std::string_view bus) const {
		std::optional<const Bus*> bus_to_process = FindBus(bus);
		if (bus_to_process.has_value()) {
			return &bus_to_process.value()->buses_wd;
		}
		return std::nullopt;
	}

	std::optional<const std::set<Bus*, Compartor>*> TransportCatalogue::ReturnBusesForStop(std::string_view stop) const {
		if (stops.contains(stop)) {
			return &stops.at(stop);
		}
		return std::nullopt;
	}

	std::size_t TransportCatalogue::ComputeActualLength(std::string_view bus) const {
		std::size_t actual_distance = 0;
		std::optional<const std::deque<Stop*>*> stops = ReturnStopsForBus(bus).value();

		for (auto it = stops.value()->rbegin(); it != stops.value()->rend() - 1; ++it) {
			if (it == stops.value()->rend() - 1) {
				Stop* last_stop = *it;
				actual_distance += last_stop->destinations.at(last_stop->name);
				break;
			}

			Stop* current_stop = *it;
			Stop* previous_stop = *(it + 1);
			actual_distance += previous_stop->destinations.at(current_stop->name);
		}

		return actual_distance;
	}

	double TransportCatalogue::ComputePureLength(std::string_view bus) const {
		double pure_distance = 0.0;
		geo::Coordinates first, second;

		bool is_first = true;
		for (const auto& stop : *ReturnStopsForBus(bus).value()) {
			first = stop->coordinates;
			if (!is_first) {
				pure_distance += geo::ComputeDistance(first, second);
			}
			second = first;
			is_first = false;
		}

		return pure_distance;
	}
}