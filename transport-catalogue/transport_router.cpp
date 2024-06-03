#include <algorithm>
#include <ranges>

#include "transport_router.h"

namespace transport_router {
// ------------ [Transport Router] Realization ------------
//                                                        +
//                                                        + ----------------
// -------------------------------------------------------- Adding methods +

	TransportRouter::TransportRouter()
		: graph_(MAX_ENTITIES_COUNT) {
	}

	void TransportRouter::FillRouter() {
		FillGraph();
		router_ = std::make_unique<graph::Router<double>>(graph_);
	}

	void TransportRouter::CreateTransfers() {
		std::size_t index = 0;

		for (auto it = deque_stops_.begin(); it != deque_stops_.end(); ++it, index += 2) {
			graph_stops_.push_back(&*it);
			graph_stops_.push_back(&*it);

			stop_indexes_[it->name] = index;
		}

		for (std::size_t index = 0; index != graph_stops_.size(); ++index) {
			graph_.AddEdge(graph::Edge<double> {
				.from = index,
				.to = ++index,
				.weight = routing_settings_.bus_wait_time * 1.0
			});
		}
	}

	void TransportRouter::FillGraph() {
		CreateTransfers();

		for (std::size_t deque_index : std::views::iota(0ULL, deque_buses_.size())) {

			if (!deque_buses_[deque_index].is_roundtrip) {
				auto forward_range_begin = deque_buses_[deque_index].stops_with_duplicates.begin();
				auto forward_range_end = deque_buses_[deque_index].stops_with_duplicates.begin() + deque_buses_[deque_index].stops_with_duplicates.size() / 2;
				CreateEdges(forward_range_begin, forward_range_end, deque_buses_[deque_index].name);

				auto backward_range_begin = deque_buses_[deque_index].stops_with_duplicates.begin() + deque_buses_[deque_index].stops_with_duplicates.size() / 2;
				auto backward_range_end = deque_buses_[deque_index].stops_with_duplicates.end() - 1;
				CreateEdges(backward_range_begin, backward_range_end, deque_buses_[deque_index].name);
			}
			else {
				auto round_range_begin = deque_buses_[deque_index].stops_with_duplicates.begin();
				auto round_range_end = deque_buses_[deque_index].stops_with_duplicates.end() - 1;

				CreateEdges(round_range_begin, round_range_end, deque_buses_[deque_index].name);
			}
		}
	}

	void TransportRouter::SetRoutingSettings(const domain::RoutingSettings& routing_settings_initializer) {
		routing_settings_ = routing_settings_initializer;
	}

// 
// 
//                                                        + --------------------
// -------------------------------------------------------- Retrieving methods +

	const graph::DirectedWeightedGraph<double>& TransportRouter::GetGraph() const {
		return graph_;
	}

	graph::Router<double>* TransportRouter::GetRouter() const {
		return router_.get();
	}

	const domain::RoutingSettings& TransportRouter::GetRoutingSettings() const {
		return routing_settings_;
	}

	const domain::SpanInfo TransportRouter::GetShortestSpan(std::string_view from, std::string_view to) {
		return domain::SpanInfo{ spans_.at(std::make_pair(from, to)).begin()->first, &spans_.at(std::make_pair(from, to)).begin()->second };
	}

	std::size_t TransportRouter::GetStopIndex(std::string_view stop) const {
		return stop_indexes_.at(stop);
	}

	const std::string& TransportRouter::GetStopNameByIndex(std::size_t index) {
		return graph_stops_.at(index)->name;
	}
} // namespace transport_router