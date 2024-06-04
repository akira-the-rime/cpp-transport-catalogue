#include <ranges>

#include "transport_router.h"

namespace transport_router {
// ------------ [Transport Router] Realization ------------
//                                                        +
//                                                        + ----------------
// -------------------------------------------------------- Adding methods +

	TransportRouter::TransportRouter(catalogue::TransportCatalogue& database, const domain::RoutingSettings& routing_settings)
		: database_(database) 
		, routing_settings_(routing_settings)
		, graph_(database.GetAllStops().size() * 2) {

		FillRouter();
	}

	void TransportRouter::CreateTransfers() {
		std::size_t index = 0;

		for (auto it = database_.GetAllStops().begin(); it != database_.GetAllStops().end(); ++it, index += 2) {
			graph_stops_.push_back(*it);
			graph_stops_.push_back(*it);

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

		for (std::size_t deque_index : std::views::iota(0ULL, database_.GetAllBuses().size())) {
			const std::vector<domain::Stop*>* stops_with_duplicates = &database_.GetAllBuses()[deque_index].stops_with_duplicates;

			if (!database_.GetAllBuses()[deque_index].is_roundtrip) {
				auto forward_range_begin = stops_with_duplicates->begin();
				auto forward_range_end = stops_with_duplicates->begin() + stops_with_duplicates->size() / 2;
				FillGraphWithBuses(forward_range_begin, forward_range_end, database_.GetAllBuses()[deque_index].name);

				auto backward_range_begin = stops_with_duplicates->begin() + stops_with_duplicates->size() / 2;
				auto backward_range_end = stops_with_duplicates->end() - 1;
				FillGraphWithBuses(backward_range_begin, backward_range_end, database_.GetAllBuses()[deque_index].name);
			}
			else {
				auto round_range_begin = stops_with_duplicates->begin();
				auto round_range_end = stops_with_duplicates->end() - 1;

				FillGraphWithBuses(round_range_begin, round_range_end, database_.GetAllBuses()[deque_index].name);
			}
		}
	}

	void TransportRouter::FillRouter() {
		FillGraph();
		router_ = std::make_unique<graph::Router<double>>(graph_);
	}

// 
// 
//                                                        + --------------------
// -------------------------------------------------------- Retrieving methods +

	const domain::Data TransportRouter::GetDataToBuildOptimalRoute(std::string_view from, std::string_view to) {
		return domain::Data {
			.route = router_->BuildRoute(stop_indexes_.at(from), stop_indexes_.at(to)),
			.graph = graph_,
			.router = router_.get(),
			.bus_wait_time = routing_settings_.bus_wait_time,
			.graph_stops = graph_stops_,
			.spans = spans_
		};
	}
} // namespace transport_router