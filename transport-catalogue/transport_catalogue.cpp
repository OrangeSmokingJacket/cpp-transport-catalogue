#include "transport_catalogue.h"


void Stop::AddRoute(const std::string& route_name)
{
	routes.insert(route_name);
}

const std::string Stop::GetName() const
{
	return name;
}
polar_coordinates::Coordinates& Stop::GetCoordinates()
{
	return coords;
}
const polar_coordinates::Coordinates Stop::GetCoordinates() const
{
	return coords;
}
std::set<std::string> Stop::GetRoutes()
{
	return routes;
}

Stop& Stop::operator=(const Stop& other)
{
	name = other.name;
	coords = other.coords;
	return *this;
}

bool Stop::operator==(const Stop& other) const
{
	return name == other.name;
}
bool Stop::operator!=(const Stop& other) const
{
	return !(*this == other);
}
bool Stop::operator<(const Stop& other) const
{
	return name < other.name;
}
bool Stop::operator<=(const Stop& other) const
{
	return *this == other || *this < other;
}
bool Stop::operator>(const Stop& other) const
{
	return other.name < name;
}
bool Stop::operator>=(const Stop& other) const
{
	return *this == other || *this > other;
}


std::string Route::GetName()
{
	return name;
}
const std::vector<std::string>& Route::GetStops() const
{
	return stops;
}
Route::RouteType Route::GetRouteType() const
{
	return type;
}
int Route::GetStopsCount() const
{
	if (type == RouteType::Curcular)
		return stops.size();
	else
		return stops.size() * 2 - 1;
}
int Route::GetRoutesUniqueStops()
{
	// for small vectors this is faster than use sort-unique-erase
	std::set<std::string> unique;
	for (const std::string& stop : stops)
	{
		unique.insert(stop);
	}
	return unique.size();
}

bool Route::operator==(const Route& other) const
{
	return name == other.name;
}
bool Route::operator!=(const Route& other) const
{
	return !(*this == other);
}
bool Route::operator<(const Route& other) const
{
	return name < other.name;
}
bool Route::operator<=(const Route& other) const
{
	return *this == other || *this < other;
}
bool Route::operator>(const Route& other) const
{
	return other.name < name;
}
bool Route::operator>=(const Route& other) const
{
	return *this == other || *this > other;
}


void TransportCatalogue::AddStop(const std::string& name, const polar_coordinates::Coordinates& coords)
{
	if (all_stops.contains(name))
		all_stops.at(name).GetCoordinates() = coords;
	else
		all_stops.insert({ name, { name, coords } });
}
void TransportCatalogue::AddStopsDistances(const std::string& name, std::vector<std::pair<std::string, double>> distances_to)
{
	for (const auto [stop, dst] : distances_to)
	{
		distances.insert({ {name, stop}, dst });
	}
}
void TransportCatalogue::AddRoute(const std::string& name, const std::vector<std::string>& stops_names, bool circular)
{
	for (const std::string& stop : stops_names)
	{
		// If there are no such stop, add a place holder
		if (!all_stops.contains(stop))
			all_stops.insert({ stop, { stop } });

		all_stops.at(stop).AddRoute(name);
	}
	all_routes.insert({ name, { std::move(name), std::move(stops_names), static_cast<Route::RouteType>(circular) } });
}

Stop TransportCatalogue::GetStop(const std::string& name)
{
	if (all_stops.contains(name))
		return all_stops.at(name);
	else
		return { "" };
}
Route TransportCatalogue::GetRoute(const std::string& name)
{
	if (all_routes.contains(name))
		return all_routes.at(name);
	else
		return {};
}

double TransportCatalogue::GetDistanceBetweenStops(const std::string& name_a, const std::string& name_b) const
{
	if (distances.contains({ name_a, name_b }))
		return distances.at({ name_a, name_b });
	else if (distances.contains({ name_b, name_a }))
		return distances.at({ name_b, name_a });
	else
		return ComputeDistance(all_stops.at(name_a).GetCoordinates(), all_stops.at(name_b).GetCoordinates());
}

double TransportCatalogue::CalculateRouteLength(const std::string& name) const
{
	double result = 0.0;
	std::vector<std::string> stops_names = all_routes.at(name).GetStops();

	// It could be one if and two cicles or one cicle and stops_names.size() if's

	if (all_routes.at(name).GetRouteType() == Route::Linear)
	{
		for (int i = 1; i < stops_names.size(); i++)
		{
			result += GetDistanceBetweenStops(stops_names[i - 1], stops_names[i]);
			result += GetDistanceBetweenStops(stops_names[i], stops_names[i - 1]);
		}
	}
	else
	{
		for (int i = 1; i < stops_names.size(); i++)
		{
			result += GetDistanceBetweenStops(stops_names[i - 1], stops_names[i]);
		}
	}

	return result;
}
double TransportCatalogue::CalculateRouteLength_RAW(const std::string& name) const
{
	double result = 0.0;
	std::vector<std::string> stops_names = all_routes.at(name).GetStops();
	for (int i = 1; i < stops_names.size(); i++)
	{
		result += ComputeDistance(all_stops.at(stops_names[i - 1]).GetCoordinates(), all_stops.at(stops_names[i]).GetCoordinates());
	}
	if (all_routes.at(name).GetRouteType() == Route::Linear)
		result *= 2;

	return result;
}