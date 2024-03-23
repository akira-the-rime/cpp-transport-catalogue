#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

#include "transport_catalogue.h"

namespace catalogue {
	void TransportCatalogue::AddStop(const std::string& name, const geo::Coordinates& coordinates) {
		deque_stops.push_back({ name, coordinates });
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

	void TransportCatalogue::OutputBusInfo(std::string_view query, 
		std::string_view bus, 
		std::ostream& output) const {
		using namespace std::literals;

		std::optional<const std::deque<catalogue::Stop*>*> to_deem = ReturnStopsForBus(bus);

		if (!to_deem.has_value()) {
			output << query << " "s << bus << ": not found" << std::endl;
			return;
		}

		output << query << " "s << bus << ": ";
		output << to_deem.value()->size() << " stops on route, "s;
		output << ReturnAmoutOfUniqueStopsForBus(bus) << " unique stops, "s;
		output << std::setprecision(6) << ComputeLength(bus) << " route length"s << std::endl;
	}

	void TransportCatalogue::OutputStopInfo(std::string_view query, 
		std::string_view stop, 
		std::ostream& output) const {
		using namespace std::literals;

		const std::optional<const std::set<catalogue::Bus*, catalogue::Compartor>*> to_deem = ReturnBusesForStop(stop);

		if (!to_deem.has_value()) {
			output << query << " "s << stop << ": not found" << std::endl;
			return;
		}
		else if (!to_deem.value()->size()) {
			output << query << " "s << stop << ": no buses" << std::endl;
			return;
		}

		bool is_first = true;
		output << query << " "s << stop << ": buses "s;
		for (const auto& bus : *to_deem.value()) {
			if (is_first) {
				output << bus->name;
				is_first = false;
				continue;
			}
			output << " "s << bus->name;
		}
		output << std::endl;
	}

	std::size_t TransportCatalogue::ReturnAmoutOfUniqueStopsForBus(std::string_view name) const {
		return buses.at(name).size();
	}

	// Перегрузка метода FindBus() для метода ReturnStopsForBus()
	std::optional<const Bus*> TransportCatalogue::FindBus(std::string_view bus) const {
		auto it = std::find(deque_buses.begin(), deque_buses.end(), bus);
		if (it != deque_buses.end()) {
			return &*it;
		}
		return std::nullopt;
	}

	std::optional<const std::deque<Stop*>*> TransportCatalogue::ReturnStopsForBus(std::string_view name) const {
		std::optional<const Bus*> bus_to_process = FindBus(name);
		if (!bus_to_process.has_value()) {
			return std::nullopt;
		}
		return &bus_to_process.value()->buses_wd;
	}

	std::optional<const std::set<Bus*, Compartor>*> TransportCatalogue::ReturnBusesForStop(std::string_view name) const {
		if (stops.count(name)) {
			return &stops.at(name);
		}
		return std::nullopt;
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