#include <algorithm>
#include <iomanip>

#include "transport_catalogue.h"

namespace catalogue {
	void TransportCatalogue::AddStop(const std::string& stop, const geo::Coordinates& coordinates) {
		deque_stops.push_back({ stop, coordinates });
		stops[deque_stops.back().name];
	}

	void TransportCatalogue::AddBus(const std::string& bus, const std::vector<std::string_view>& proper_stops) {
		deque_buses.push_back({ bus, std::deque<Stop*>{} });

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

	std::stringstream TransportCatalogue::GetBusInfo(std::string_view bus) const {
		using namespace std::literals;

		std::stringstream output;
		std::optional<const std::deque<catalogue::Stop*>*> to_deem = ReturnStopsForBus(bus);

		if (!to_deem.has_value()) {
			output << "Bus "s << bus << ": not found"s;
			return output;
		}

		output << "Bus "s << bus << ": "s;
		output << to_deem.value()->size() << " stops on route, "s;
		output << ReturnAmoutOfUniqueStopsForBus(bus) << " unique stops, "s;
		output << std::setprecision(6) << ComputeLength(bus) << " route length"s;
		return output;
	}

	std::stringstream TransportCatalogue::GetStopInfo(std::string_view stop) const {
		using namespace std::literals;

		std::stringstream output;
		const std::optional<const std::set<catalogue::Bus*, catalogue::Compartor>*> to_deem = ReturnBusesForStop(stop);

		if (!to_deem.has_value()) {
			output << "Stop "s << stop << ": not found"s;
			return output;
		}
		else if (!to_deem.value()->size()) {
			output << "Stop "s << stop << ": no buses"s;
			return output;
		}

		bool is_first = true;
		output << "Stop "s << stop << ": buses "s;
		for (const auto& bus : *to_deem.value()) {
			if (is_first) {
				output << bus->name;
				is_first = false;
				continue;
			}
			output << " "s << bus->name;
		}
		return output;
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
		if (!bus_to_process.has_value()) {
			return std::nullopt;
		}
		return &bus_to_process.value()->buses_wd;
	}

	std::optional<const std::set<Bus*, Compartor>*> TransportCatalogue::ReturnBusesForStop(std::string_view stop) const {
		if (stops.count(stop)) {
			return &stops.at(stop);
		}
		return std::nullopt;
	}

	double TransportCatalogue::ComputeLength(std::string_view bus) const {
		double distance = 0.0;
		geo::Coordinates first, second;

		bool is_first = true;
		for (const auto& stop : *ReturnStopsForBus(bus).value()) {
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