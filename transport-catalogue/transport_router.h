#pragma once

#include <concepts>
#include <memory>

#include "domain.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

namespace transport_router {
// ------------ [Transport Router] Definition ------------
//                                                       +
//                                                       + ------------------
// ------------------------------------------------------- Transport Router +

	class TransportRouter final {
	public:
		TransportRouter(catalogue::TransportCatalogue& database, const domain::RoutingSettings& routing_settings);
		const domain::Data GetDataToBuildOptimalRoute(std::string_view from, std::string_view to);
		
	private:
		void CreateTransfers();
		void FillGraph();
		void FillRouter();

		template <std::random_access_iterator RandomIt>
		void FillGraphWithBuses(RandomIt stop_range_begin, RandomIt stop_range_end, const std::string& bus_name);

		std::unordered_map<std::pair<std::string_view, std::string_view>, std::map<std::size_t, std::deque<std::string_view>>, domain::Hasher> spans_;
		std::unordered_map<std::string_view, std::size_t> stop_indexes_;

		catalogue::TransportCatalogue& database_;
		domain::RoutingSettings routing_settings_;
		graph::DirectedWeightedGraph<double> graph_;

		std::deque<domain::Stop> graph_stops_;
		std::unique_ptr<graph::Router<double>> router_;
	};

// 
// 
//                                                       + --------------------------
// ------------------------------------------------------- Filling Graph with Buses +

	template <std::random_access_iterator RandomIt>
	void TransportRouter::FillGraphWithBuses(RandomIt global_begin, RandomIt global_end, const std::string& bus_name) {
		auto stop_range_begin = global_begin;
		auto stop_range_end = global_end;

		domain::Stop* beginning_stop = *stop_range_begin;
		std::size_t beginning_stop_index = stop_indexes_.at(beginning_stop->name);

		while (global_begin != global_end) {
			const domain::Stop* ending_stop = *stop_range_end;
			const std::size_t ending_stop_index = stop_indexes_.at(ending_stop->name);
			auto stop_iterator = stop_range_begin;
			double length = {};

			while (stop_iterator != stop_range_end) {
				const domain::Stop* next_stop = *std::ranges::next(stop_iterator, 1);
				length += database_.GetDestinations().at(std::make_pair((*stop_iterator)->name, next_stop->name));

				std::ranges::advance(stop_iterator, 1);
			}

			double weight = length / 1000.0 / routing_settings_.bus_velocity * 60;
			graph_.AddEdge(graph::Edge<double> {
				.from = beginning_stop_index + 1,
				.to = ending_stop_index,
				.weight = weight
			});
			spans_[{ beginning_stop->name, ending_stop->name }][static_cast<std::size_t>(std::ranges::distance(stop_range_begin, stop_range_end))].push_back(bus_name);

			std::ranges::advance(stop_range_end, -1);

			if (stop_range_begin == stop_range_end) {
				std::ranges::advance(global_begin, 1);

				stop_range_begin = global_begin;
				stop_range_end = global_end;

				beginning_stop = *stop_range_begin;
				beginning_stop_index = stop_indexes_.at(beginning_stop->name);
			}
		}
	}
} // namespace transport_router