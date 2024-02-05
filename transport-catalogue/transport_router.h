#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

namespace trans_cat
{
	using TransportGraph = graph::DirectedWeightedGraph<double>;
	class TransportRouter
	{
	public:

		void BuildGraph(const TransportCatalogue& catalogue);

		const TransportGraph& GetGraph(void);

	private:
		TransportGraph graph_
	};
} // namespace trans_cat
