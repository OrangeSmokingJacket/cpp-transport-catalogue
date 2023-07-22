#include "transport_catalogue.h"

void TransportCatalogue::AddStop(const std::string& name, const polar_coordinates::Coordinates& coords)
{
	if (all_stops.count(name) != 0)
		all_stops.at(name).SetCoordinates(coords);
	else
		all_stops.insert({ name, { name, coords } });
}
void TransportCatalogue::AddStopsDistances(const std::string& from, std::vector<std::pair<std::string, double>> distances_to)
{
	for (const auto& [stop, dst] : distances_to)
	{
		distances.insert({ { from, stop }, dst });
	}
}
void TransportCatalogue::AddStopsDistances(const std::string& from, const std::string& to, double dst)
{
	distances.insert({ { from, to }, dst });
}
void TransportCatalogue::AddRoute(const std::string& name, const std::vector<std::string>& stops_names, bool circular)
{
	std::vector<Stop*> stops;
	for (const std::string& stop : stops_names)
	{
		// If there are no such stop, add a place holder
		if (all_stops.count(stop) == 0)
			all_stops.insert({ stop, { stop } });

		stops.push_back(&(all_stops.at(stop)));
	}

	all_routes.insert({ name, { std::move(name), std::move(stops), static_cast<Route::RouteType>(circular) } });

	Route* route_ptr = &(all_routes.at(name));
	for (const std::string& stop : stops_names)
	{
		all_stops.at(stop).AddRoute(route_ptr);
	}
}

std::optional<Stop> TransportCatalogue::GetStop(const std::string& name)
{
	std::optional<Stop> empty;
	if (all_stops.count(name))
		return all_stops.at(name);
	else
		return empty;
}
std::optional<Route> TransportCatalogue::GetRoute(const std::string& name)
{
	std::optional<Route> empty;
	if (all_routes.count(name))
		return all_routes.at(name);
	else
		return empty;
}
std::vector<Route*> TransportCatalogue::GetAllRoutes()
{
	std::vector<Route*> result;
	result.reserve(all_routes.size());

	for (const auto route : all_routes)
	{
		result.emplace_back(&all_routes.at(route.first)); // &(route.second) is not compiling
	}
	std::sort(result.begin(), result.end(), [](Route* lhs, Route* rhs) { return lhs->GetName() < rhs->GetName(); });
	return result;
}
const std::unordered_map<std::string, Stop>& TransportCatalogue::GetStopsRef() const
{
	return all_stops;
}
const std::unordered_map<std::string, Route>& TransportCatalogue::GetRoutesRef() const
{
	return all_routes;
}
const std::unordered_map<std::pair<std::string, std::string>, double, StopPairHasher>& TransportCatalogue::GetDistancesRef() const
{
	return distances;
}

double TransportCatalogue::GetDistanceBetweenStops(const std::string& name_a, const std::string& name_b) const
{
	if (distances.count({ name_a, name_b }))
		return distances.at({ name_a, name_b });
	else if (distances.count({ name_b, name_a }))
		return distances.at({ name_b, name_a });
	else
		return ComputeDistance(all_stops.at(name_a).GetCoordinates(), all_stops.at(name_b).GetCoordinates());
}

double TransportCatalogue::CalculateRouteLength(const std::string& name) const
{
	double result = 0.0;
	const std::vector<Stop*>& stops = all_routes.at(name).GetStops();

	// It could be one if and two cicles or one cicle and stops_names.size() if's

	if (all_routes.at(name).GetRouteType() == Route::Linear)
	{
		for (size_t i = 1; i < stops.size(); i++)
		{
			result += GetDistanceBetweenStops(stops[i - 1]->GetName(), stops[i]->GetName());
			result += GetDistanceBetweenStops(stops[i]->GetName(), stops[i - 1]->GetName());
		}
	}
	else
	{
		for (size_t i = 1; i < stops.size(); i++)
		{
			result += GetDistanceBetweenStops(stops[i - 1]->GetName(), stops[i]->GetName());
		}
	}

	return result;
}
double TransportCatalogue::CalculateRouteLength_RAW(const std::string& name) const
{
	double result = 0.0;
	const std::vector<Stop*>& stops = all_routes.at(name).GetStops();
	for (size_t i = 1; i < stops.size(); i++)
	{
		result += ComputeDistance(stops[i - 1]->GetCoordinates(), stops[i]->GetCoordinates());
	}
	if (all_routes.at(name).GetRouteType() == Route::Linear)
		result *= 2;

	return result;
}