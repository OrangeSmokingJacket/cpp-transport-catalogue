#pragma once

#include "geo.h"

#include <vector>
#include <set>


class Route;
class Stop
{
public:
	Stop(const std::string& name_) : name(name_) {}
	Stop(const std::string& name_, polar_coordinates::Coordinates coords_) : name(name_), coords(coords_) {}
	Stop(const std::string& name_, double longitude, double latitude) : name(name_), coords({ longitude , latitude }) {}
	Stop(const Stop& other)
	{
		name = other.name;
		coords = other.coords;
		routes = other.routes;
	}

	void AddRoute(Route* route);
	void SetCoordinates(polar_coordinates::Coordinates coords_);

	const std::string GetName() const;
	const polar_coordinates::Coordinates GetCoordinates() const;
	std::set<Route*> GetRoutes();

	Stop& operator=(const Stop& other);

	bool operator==(const Stop& other) const;
	bool operator!=(const Stop& other) const;
	bool operator<(const Stop& other) const;
	bool operator<=(const Stop& other) const;
	bool operator>(const Stop& other) const;
	bool operator>=(const Stop& other) const;
private:
	std::string name;
	polar_coordinates::Coordinates coords;
	std::set<Route*> routes;
};
struct StopPairHasher
{
	size_t operator() (const std::pair<std::string, std::string>& names) const
	{
		return std::hash<std::string>{}(names.first) + std::hash<std::string>{}(names.second) * 37;
	}
};
class Route
{
public:
	enum RouteType { Linear = 0, Curcular = 1 };
	Route() = default;
	Route(const std::string name_, std::vector<Stop*> stops_, RouteType type_) : name(name_), stops(stops_), type(type_) {}

	std::string GetName();
	std::vector<Stop*> GetStops() const;
	RouteType GetRouteType() const;
	int GetStopsCount() const;
	int GetRoutesUniqueStops();

	bool operator==(const Route& other) const;
	bool operator!=(const Route& other) const;
	bool operator<(const Route& other) const;
	bool operator<=(const Route& other) const;
	bool operator>(const Route& other) const;
	bool operator>=(const Route& other) const;

private:
	std::string name;
	std::vector<Stop*> stops;
	RouteType type;
};