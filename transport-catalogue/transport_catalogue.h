#pragma once

#include <cstddef>
#include <optional>
#include <set>
#include <span>
#include <utility>

#include "domain.h"
#include "graph.h"

namespace catalogue {
// ------------ [Transport Catalogue] Definition ------------
//                                                          +
//                                                          + ---------------------
// ---------------------------------------------------------- Transport Catalogue +

	class TransportCatalogue final {
	public:
		void AddStop(const std::string& stop, const geo::Coordinates& coordinates);
		void AddDestination(const std::string& stop, const std::string& dst, const std::size_t length);
		void AddBus(const std::string& bus, std::span<const std::string_view> proper_stops, bool is_roundtrip);

		domain::Bus* FindBus(std::string_view bus);
		domain::Stop* FindStop(std::string_view stop);
		
		const domain::BusInfo GetBusInfo(std::string_view bus) const;
		const domain::StopInfo GetStopInfo(std::string_view stop) const;

		const std::deque<domain::Stop>* GetAllStops() const;
		const std::unordered_map<std::pair<std::string_view, std::string_view>, std::size_t, domain::Hasher>* GetDestinations() const;
		const std::deque<domain::Bus>* GetAllBuses() const;

	private:
		size_t ComputeActualLength(std::string_view bus) const;
		double ComputePureLength(std::string_view bus) const;
		std::optional<const std::set<domain::Bus*, domain::Compartor>*> GetBusesForStop(std::string_view stop) const;
		std::optional<const std::vector<domain::Stop*>*> GetStopsForBus(std::string_view bus) const;
		std::optional<const domain::Bus*> FindBus(std::string_view bus) const;
		std::size_t ReturnAmoutOfUniqueStopsForBus(std::string_view bus) const;

		std::deque<domain::Bus> deque_buses_;
		std::deque<domain::Stop> deque_stops_;
		
		std::unordered_map<std::string_view, std::unordered_set<domain::Stop*>> buses_;
		std::unordered_map<std::pair<std::string_view, std::string_view>, std::size_t, domain::Hasher> destinations_;
		std::unordered_map<std::string_view, std::set<domain::Bus*, domain::Compartor>> stops_;
	};
} // namespace catalogue