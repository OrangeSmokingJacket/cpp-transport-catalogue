#include "domain.h"


void Stop::AddRoute(Route* route)
{
	routes.insert(route);
}
void Stop::SetCoordinates(polar_coordinates::Coordinates coords_)
{
	coords = coords_;
}

const std::string Stop::GetName() const
{
	return name;
}
const polar_coordinates::Coordinates Stop::GetCoordinates() const
{
	return coords;
}
const std::set<Route*>& Stop::GetRoutes() const
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


std::string Route::GetName() const
{
	return name;
}
const std::vector<Stop*>& Route::GetStops() const
{
	return stops;
}
std::vector<Stop*> Route::GetFullRoute() const
{
	if (type == RouteType::Curcular)
		return stops;
	
	std::vector<Stop*> result(stops.size() * 2 - 1);
	std::copy(stops.begin(), stops.end(), result.begin());
	std::copy(stops.rbegin()++, stops.crend(), result.begin() + (stops.size() - 1));
	return result;
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
	std::set<Stop*> unique;
	for (Stop* stop : stops)
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