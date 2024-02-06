#include "transport_router.h"


namespace trans_cat
{

	void TransportRouter::BuildGraph(const trans_cat::TransportCatalogue& catalogue)
	{
		auto all_stopes = catalogue.GetAllStops();
		TransportGraph stop_graph(2 * all_stopes.size());
		graph::VertexId stop_id = 0;
		std::unordered_map<std::string_view, graph::VertexId> stopname_to_edge;
		auto bus_velocity = catalogue.GetVelocity();
		auto bus_wait_time = catalogue.GetWaitTime();
		for (const auto& [stop_name, stop] : all_stopes)
		{
			stopname_to_edge[stop_name] = stop_id;
			stop_graph.AddEdge({stop_id, ++stop_id, static_cast<double>(bus_wait_time)});
			id_to_busname_[graph_.GetEdgeCount()] = std::string_view{};
			stop_id++;
		}
		auto& all_buses = catalogue.GetSortedAllBuses();
		for (const auto& [bus_name, bus] : all_buses)
		{
			auto& stops = bus->stops;
			for (int i = 0; i < stops.size() - 1; i++)
			{
				for (int j = i + 1; j < stops.size() ; j++)
				{
					auto stop_from = stops[i]->stop_name;
					auto stop_to = stops[j]->stop_name;
					double dist_sum = 0;
					double dist_inverse_sum = 0;
					for (int k = i; k < j; k++)
					{
						dist_sum += catalogue.GetDistance(stops[k]->stop_name, stops[k+1]->stop_name);
						dist_inverse_sum += catalogue.GetDistance(stops[k + 1]->stop_name, stops[k]->stop_name);
					}
					auto id_from = stopname_to_edge.at(stop_from);
					auto id_to = stopname_to_edge.at(stop_to);
					stop_graph.AddEdge({ id_from + 1, id_to, dist_sum / trans_cat::KMPH_TO_MPM });
					id_to_busname_[graph_.GetEdgeCount()] = bus_name;
					if (!bus->is_sircle)
					{
						stop_graph.AddEdge({id_to + 1, id_from, dist_inverse_sum / trans_cat::KMPH_TO_MPM });
						id_to_busname_[graph_.GetEdgeCount()] = bus_name;
					}
				}
			}
		}
		graph_ = std::move(stop_graph);
		stopname_ids_ = std::move(stopname_to_edge);
		router_ = std::make_unique<graph::Router<double>>(graph_);
	}

	std::optional<graph::Router<double>::RouteInfo> TransportRouter::GetStopRoute(std::string_view stop_from, std::string_view stop_to) const
	{
		auto id_from = stopname_ids_.at(stop_from);
		auto id_to = stopname_ids_.at(stop_to);
		return router_->BuildRoute(id_from, id_to);
	}

	const TransportGraph& TransportRouter::GetGraph(void)
	{
		return graph_;
	}

	std::string_view TransportRouter::GetEdgeName(graph::VertexId id)
	{
		if (id_to_busname_.count(id)) {
			return id_to_busname_.at(id);
		}
		return std::string_view{};
	}
} // namespace trans_cat