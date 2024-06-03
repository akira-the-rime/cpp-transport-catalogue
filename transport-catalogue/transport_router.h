#pragma once

#include <concepts>
#include <cstddef>
#include <memory>
#include <map>

#include "domain.h"
#include "graph.h"
#include "router.h"

namespace transport_router {
// ------------ [Transport Router] Definition ------------
//                                                       +
//                                                       + ------------------
// ------------------------------------------------------- Transport Router +

	class TransportRouter final {
	public:
		TransportRouter();
		void FillRouter();

		const graph::DirectedWeightedGraph<double>& GetGraph() const;
		graph::Router<double>* GetRouter() const;
		const domain::RoutingSettings& GetRoutingSettings() const;
		const domain::SpanInfo GetShortestSpan(std::string_view from, std::string_view to);
		std::size_t GetStopIndex(std::string_view stop) const;
		const std::string& GetStopNameByIndex(std::size_t index);
		
		void SetRoutingSettings(const domain::RoutingSettings& routing_settings_initializer);
		
	private:
		void CreateTransfers();
		void FillGraph();

		std::deque<domain::Bus> deque_buses_;
		std::unordered_map<std::pair<std::string_view, std::string_view>, std::size_t, domain::Hasher> destinations_;
		std::deque<domain::Stop> deque_stops_;

		std::unordered_map<std::pair<std::string_view, std::string_view>, std::map<std::size_t, std::deque<std::string_view>>, domain::Hasher> spans_;
		std::unordered_map<std::string_view, std::size_t> stop_indexes_;

		graph::DirectedWeightedGraph<double> graph_;
		std::deque<domain::Stop*> graph_stops_;
		std::unique_ptr<graph::Router<double>> router_;

		domain::RoutingSettings routing_settings_;
		static const std::size_t MAX_ENTITIES_COUNT = 200;

	public:
		template <std::random_access_iterator RandomIt>
		void SetStops(RandomIt begin, RandomIt end);

		template <std::random_access_iterator RandomIt>
		void SetBuses(RandomIt begin, RandomIt end);

		template <std::forward_iterator ForwardIt>
		void SetDesinations(ForwardIt begin, ForwardIt end);

	private:
		template <std::random_access_iterator RandomIt>
		void CreateEdges(RandomIt stop_range_begin, RandomIt stop_range_end, const std::string& bus_name);
	};

// 
// 
//                                                       + ------------------
// ------------------------------------------------------- Template Methods +

	template <std::random_access_iterator RandomIt>
	void TransportRouter::SetStops(RandomIt begin, RandomIt end) {
		std::size_t distance = std::ranges::distance(begin, end);
		deque_stops_.resize(distance);

		for (std::size_t index = 0; index != distance; ++index, ++begin) {
			deque_stops_[index] = *begin;
		}
	}

	template <std::random_access_iterator RandomIt>
	void TransportRouter::SetBuses(RandomIt begin, RandomIt end) {
		std::size_t distance = std::ranges::distance(begin, end);
		deque_buses_.resize(distance);

		for (std::size_t index = 0; index != distance; ++index, ++begin) {
			deque_buses_[index] = *begin;
		}
	}

	template <std::forward_iterator ForwardIt>
	void TransportRouter::SetDesinations(ForwardIt begin, ForwardIt end) {
		while (begin != end) {
			destinations_[begin->first] = begin->second;

			++begin;
		}
	}

	template <std::random_access_iterator RandomIt>
	void TransportRouter::CreateEdges(RandomIt global_begin, RandomIt global_end, const std::string& bus_name) {
		auto stop_range_begin = global_begin;
		auto stop_range_end = global_end;

		domain::Stop* beginning_stop = *stop_range_begin;
		std::size_t beginning_stop_index = GetStopIndex(beginning_stop->name);

		while (global_begin != global_end) {
			const domain::Stop* ending_stop = *stop_range_end;
			const std::size_t ending_stop_index = GetStopIndex(ending_stop->name);
			auto stop_iterator = stop_range_begin;
			double length = {};

			while (stop_iterator != stop_range_end) {
				const domain::Stop* next_stop = *std::ranges::next(stop_iterator, 1);
				length += destinations_.at(std::make_pair((*stop_iterator)->name, next_stop->name));

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
				beginning_stop_index = GetStopIndex(beginning_stop->name);
			}
		}
	}
} // namespace transport_router