#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <map>
#include <memory>

class TransportRouter
{
public:
	TransportRouter(TransportCatalogue& catalogue, double speed, int waiting_time) : catalogue(catalogue), bus_speed(speed), stop_waiting_time(waiting_time),
		all_stops(catalogue.GetStopsRef()), all_routes(catalogue.GetRoutesRef()), routes_graph(all_stops.size())
	{
		ConstructIndexMap();
		ConstructGraph();
	}
	std::optional<graph::Router<double>::RouteInfo> FindPath(const std::string& from, const std::string& to);

	size_t GetIndex(const std::string& stop_name);
	std::string GetStopName(size_t index);
	graph::Edge<double> GetEdge(graph::EdgeId id);
	std::pair<std::string, size_t> GetEdgeData(graph::EdgeId id);
	int GetWaitingTime() const;

private:
	void ConstructIndexMap();
	void ConstructGraph();

	TransportCatalogue& catalogue;
	double bus_speed;
	int stop_waiting_time;
	const std::unordered_map<std::string, Stop>& all_stops;
	const std::unordered_map<std::string, Route>& all_routes;

	std::map<std::string, size_t> index_from_stop;
	std::map<size_t, std::string> stop_from_index;
	std::map<graph::EdgeId, std::pair<std::string, size_t>> bus_span_from_edge;

	graph::DirectedWeightedGraph<double> routes_graph;
	std::unique_ptr<graph::Router<double>> router;
};