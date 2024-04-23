#pragma once

#include <cstddef>
#include <deque>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "domain.h"

namespace catalogue {
// ------------------------------ [Transport Catalogue] Definition --------------------
//                                                                                    +
//                                                                                    + ---------------------
// ------------------------------------------------------------------------------------ Transport Catalogue +

	class TransportCatalogue final {
	public:
		void AddStop(const std::string& stop, const geo::Coordinates& coordinates);
		void AddDestination(const std::string& stop, const std::string& dst, const std::size_t length);
		void AddBus(const std::string& bus, const std::vector<std::string_view>& proper_stops);

		domain::Stop* FindStop(std::string_view stop);
		domain::Bus* FindBus(std::string_view bus);
		
		const domain::BusInfo GetBusInfo(std::string_view bus) const;
		const domain::StopInfo GetStopInfo(std::string_view stop) const;
	private:
		std::size_t ReturnAmoutOfUniqueStopsForBus(std::string_view bus) const;
		std::optional<const domain::Bus*> FindBus(std::string_view bus) const;
		std::optional<const std::vector<domain::Stop*>*> ReturnStopsForBus(std::string_view bus) const;
		std::optional<const std::set<domain::Bus*, domain::Compartor>*> ReturnBusesForStop(std::string_view stop) const;
		size_t ComputeActualLength(std::string_view bus) const;
		double ComputePureLength(std::string_view bus) const;

		std::deque<domain::Stop> deque_stops;
		std::deque<domain::Bus> deque_buses;

		std::unordered_map<std::string_view, std::set<domain::Bus*, domain::Compartor>> stops;
		std::unordered_map<std::pair<std::string_view, std::string_view>, std::size_t, domain::Hasher> destinations;
		std::unordered_map<std::string_view, std::unordered_set<domain::Stop*>> buses;
	};
} // namespace catalogue