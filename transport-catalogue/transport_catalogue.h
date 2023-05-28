#pragma once

#include "domain.h"

#include <unordered_map>
#include <optional>
#include <algorithm>


class TransportCatalogue
{
public:
	void AddStop(const std::string& name, const polar_coordinates::Coordinates& coords);
	void AddStopsDistances(const std::string& name, std::vector<std::pair<std::string, double>> distances_to);
	void AddRoute(const std::string& name, const std::vector<std::string>& stops_names, bool circular);

	std::optional<Stop> GetStop(const std::string& name);
	std::optional<Route> GetRoute(const std::string& name);
	std::vector<Route*> GetAllRoutes();

	double GetDistanceBetweenStops(const std::string& name_a, const std::string& name_b) const;

	double CalculateRouteLength(const std::string& name) const;
	double CalculateRouteLength_RAW(const std::string& name) const;
private:
	std::unordered_map<std::string, Stop> all_stops;
	std::unordered_map<std::string, Route> all_routes;
	std::unordered_map<std::pair<std::string, std::string>, double, StopPairHasher> distances;
};