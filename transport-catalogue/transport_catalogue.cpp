#include <algorithm>

#include "transport_catalogue.h"

namespace catalogue {
// ------------ [Transport Catalogue] Realization ------------
//                                                           +
//                                                           + ----------------
// ----------------------------------------------------------- Adding methods +

	void TransportCatalogue::AddStop(const std::string& stop, const geo::Coordinates& coordinates) {
		deque_stops_.emplace_back(stop, coordinates);
		stops_[deque_stops_.back().name];
	}

	void TransportCatalogue::AddDestination(const std::string& stop, const std::string& dst, 
		const std::size_t length) {

		domain::Stop* stop_to_process = FindStop(stop);
		domain::Stop* dst_to_process = FindStop(dst);
		destinations_[{ stop_to_process->name, dst_to_process->name }] = length;

		if (!destinations_.contains(std::make_pair(dst_to_process->name, stop_to_process->name))) {
			destinations_[{ dst_to_process->name, stop_to_process->name }] = length;
		}
	}

	void TransportCatalogue::AddBus(const std::string& bus, std::span<const std::string_view> proper_stops, 
		bool is_roundtrip) {

		deque_buses_.emplace_back(bus, std::vector<domain::Stop*>{}, is_roundtrip);
		domain::Bus* bus_to_process = &deque_buses_.back();

		for (const auto& stop : proper_stops) {
			stops_.at(stop).insert(bus_to_process);

			domain::Stop* stop_to_process = FindStop(stop);
			buses_[bus_to_process->name].insert(stop_to_process);

			FindBus(bus)->stops_with_duplicates.push_back(stop_to_process);
		}
	}

//
// 
//                                                           + --------------------
// ----------------------------------------------------------- Retrieving methods +

	domain::Bus* TransportCatalogue::FindBus(std::string_view bus) {
		return &*std::ranges::find(deque_buses_, bus, [](const domain::Bus& bus) {
			return bus.name;
		});
	}

	domain::Stop* TransportCatalogue::FindStop(std::string_view stop) {
		return &*std::ranges::find(deque_stops_, stop, [](const domain::Stop& range_stop) {
			return range_stop.name;
		});
	}

	const domain::BusInfo TransportCatalogue::GetBusInfo(std::string_view bus) const {
		using namespace std::literals;

		domain::BusInfo to_output;
		std::optional<const std::vector<domain::Stop*>*> to_deem = GetStopsForBus(bus);

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

	const domain::StopInfo TransportCatalogue::GetStopInfo(std::string_view stop) const {
		using namespace std::literals;

		domain::StopInfo to_output;
		const std::optional<const std::set<domain::Bus*, domain::Compartor>*> to_deem = GetBusesForStop(stop);

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

	const std::deque<domain::Stop>* TransportCatalogue::GetAllStops() const {
		return &deque_stops_;
	}

	const std::unordered_map<std::pair<std::string_view, std::string_view>, std::size_t, domain::Hasher>* TransportCatalogue::GetDestinations() const {
		return &destinations_;
	}

	const std::deque<domain::Bus>* TransportCatalogue::GetAllBuses() const {
		return &deque_buses_;
	}

	std::optional<const std::set<domain::Bus*, domain::Compartor>*> TransportCatalogue::GetBusesForStop(std::string_view stop) const {
		if (stops_.contains(stop)) {
			return &stops_.at(stop);
		}

		return std::nullopt;
	}

	std::optional<const domain::Bus*> TransportCatalogue::FindBus(std::string_view bus) const {
		auto it = std::ranges::find(deque_buses_, bus, [](const domain::Bus& range_bus) {
			return range_bus.name;
		});

		if (it != deque_buses_.end()) {
			return &*it;
		}

		return std::nullopt;
	}

	std::optional<const std::vector<domain::Stop*>*> TransportCatalogue::GetStopsForBus(std::string_view bus) const {
		std::optional<const domain::Bus*> bus_to_process = FindBus(bus);

		if (bus_to_process.has_value()) {
			return &bus_to_process.value()->stops_with_duplicates;
		}

		return std::nullopt;
	}

//
// 
//                                                           + -------------------
// ----------------------------------------------------------- Computing methods +

	std::size_t TransportCatalogue::ComputeActualLength(std::string_view bus) const {
		std::size_t actual_distance = 0;
		std::optional<const std::vector<domain::Stop*>*> stops = GetStopsForBus(bus).value();

		for (auto it = stops.value()->rbegin(); it != stops.value()->rend() - 1; ++it) {
			domain::Stop* current_stop = *it;
			domain::Stop* previous_stop = *(it + 1);
			actual_distance += destinations_.at(std::make_pair(previous_stop->name, current_stop->name));
		}

		return actual_distance;
	}

	double TransportCatalogue::ComputePureLength(std::string_view bus) const {
		double pure_distance = 0.0;
		geo::Coordinates first, second;

		bool is_first = true;
		for (const auto& stop : *GetStopsForBus(bus).value()) {
			first = stop->coordinates;
			if (!is_first) {
				pure_distance += geo::ComputeDistance(first, second);
			}
			second = first;
			is_first = false;
		}

		return pure_distance;
	}

	std::size_t TransportCatalogue::ReturnAmoutOfUniqueStopsForBus(std::string_view bus) const {
		return buses_.at(bus).size();
	}
} // namespace catalogue