#include "transport_router.h"

std::optional<graph::Router<double>::RouteInfo> TransportRouter::FindPath(const std::string& from, const std::string& to)
{
	if (all_stops.count(from) == 0 || all_stops.count(to) == 0)
		return std::nullopt;

	if (all_stops.at(from).GetRoutes().size() == 0 || all_stops.at(to).GetRoutes().size() == 0)
		return std::nullopt;

	return router.get()->BuildRoute(index_from_stop.at(from), index_from_stop.at(to));
}

size_t TransportRouter::GetIndex(const std::string& stop_name)
{
	return index_from_stop.at(stop_name);
}
std::string TransportRouter::GetStopName(size_t index)
{
	return stop_from_index.at(index);
}
graph::Edge<double> TransportRouter::GetEdge(graph::EdgeId id)
{
	return routes_graph.GetEdge(id);
}
std::pair<std::string, size_t> TransportRouter::GetEdgeData(graph::EdgeId id)
{
	return bus_span_from_edge.at(id);
}
int TransportRouter::GetWaitingTime() const
{
	return settings.stop_waiting_time;
}

const TransportRouterSettings& TransportRouter::GetSettings() const
{
	return settings;
}

void TransportRouter::ConstructIndexMap()
{
	size_t index = 0u;
	for (const auto& [stop_name, stop] : all_stops)
	{
		index_from_stop[stop_name] = index;
		stop_from_index[index] = stop_name;
		index++;
	}
}
void TransportRouter::ConstructGraph()
{
	for (const auto& [route_name, route] : all_routes)
	{
		const std::vector<Stop*> stops = route.GetFullRoute();

		for (size_t index_1 = 0; index_1 < stops.size(); index_1++)
		{
			double accumulated_weight = 0;
			for (size_t index_2 = index_1 + 1; index_2 < stops.size(); index_2++)
			{
				accumulated_weight += catalogue.GetDistanceBetweenStops(stops[index_2 - 1]->GetName(), stops[index_2]->GetName());
				graph::EdgeId id = routes_graph.AddEdge({ GetIndex(stops[index_1]->GetName()), GetIndex(stops[index_2]->GetName()), accumulated_weight / settings.bus_speed + settings.stop_waiting_time });
				bus_span_from_edge[id] = { route_name, index_2 - index_1 };
			}
		}
	}
	router = std::make_unique<graph::Router<double>>(routes_graph);
}