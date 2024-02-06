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
        const double to_multiply_velocity = 1.0/(bus_velocity * trans_cat::KMPH_TO_MPM); 
		auto bus_wait_time = catalogue.GetWaitTime();
		auto all_buses = catalogue.GetAllBuses();

        
        for (const auto& [stop_name, stop] : all_stopes)
		{
			stopname_to_edge[stop_name] = stop_id;
			stop_graph.AddEdge({stop_name, 0, stop_id, ++stop_id, static_cast<double>(bus_wait_time)});
			stop_id++;
		}
        
		for (const auto& [bus_name, bus] : all_buses)
		{
			auto& stops = bus->stops;
			for (std::size_t i = 0; i < (stops.size() - 1); i++)
			{
                double dist_sum = 0;
                double dist_inverse_sum = 0;
				for (std::size_t j = i + 1; j < stops.size(); j++)
				{
                    
                    
					auto stop_from = stops[i]->stop_name;
					auto stop_to = stops[j]->stop_name;
					dist_sum += catalogue.GetDistance(stops[j-1]->stop_name, stops[j]->stop_name);
				    dist_inverse_sum += catalogue.GetDistance(stops[j]->stop_name, stops[j-1]->stop_name);
					auto id_from = stopname_to_edge.at(stop_from);
					auto id_to = stopname_to_edge.at(stop_to);
					stop_graph.AddEdge({bus_name, (j - i), id_from + 1, id_to, dist_sum*to_multiply_velocity});
					if (!bus->is_sircle) {
						stop_graph.AddEdge({bus_name, (j - i),id_to + 1, id_from, dist_inverse_sum*to_multiply_velocity });
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

        if(stopname_ids_.count(stop_from) && stopname_ids_.count(stop_to)) {
            auto id_from = stopname_ids_.at(stop_from);
            auto id_to = stopname_ids_.at(stop_to);
           return router_->BuildRoute(id_from, id_to);
        }
        return std::nullopt;

	}

	const TransportGraph& TransportRouter::GetGraph(void)
	{
		return graph_;
	}

} // namespace trans_cat