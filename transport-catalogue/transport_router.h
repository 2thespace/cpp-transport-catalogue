#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

namespace trans_cat
{
	using TransportGraph = graph::DirectedWeightedGraph<double>;
	class TransportRouter
	{
	public:
		TransportRouter() = default;
		void BuildGraph(const TransportCatalogue& catalogue);
		std::string_view GetEdgeName(graph::VertexId id);
		std::optional<graph::Router<double>::RouteInfo> GetStopRoute(std::string_view stop_from, std::string_view stop_to) const;
		const TransportGraph& GetGraph(void);
	private:
		TransportGraph graph_;
		std::unordered_map<std::string_view, graph::VertexId> stopname_ids_;
		std::unordered_map<graph::VertexId, std::string_view> id_to_busname_;
		std::unique_ptr<graph::Router<double>> router_;
	};
} // namespace trans_cat
