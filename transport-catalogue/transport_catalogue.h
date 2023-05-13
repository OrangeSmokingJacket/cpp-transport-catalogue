#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <set>

#include "geo.h"


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

	void AddRoute(std::string route_name);

    const std::string GetName() const;
	polar_coordinates::Coordinates& GetCoordinates();
	const polar_coordinates::Coordinates GetCoordinates() const;
	std::set<std::string> GetRoutes();

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
	std::set<std::string> routes;
};
class Route
{
public:
	enum RouteType { Linear = 0, Curcular = 1 };
	Route() = default;
	Route(const std::string name_, const std::vector<std::string>& stops_, RouteType type_) : name(name_), stops(stops_), type(type_) {}

	std::string GetName();
	const std::vector<std::string>& GetStops() const;
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
	std::vector<std::string> stops;
	RouteType type;
};
struct StopPairHasher
{
    size_t operator() (const std::pair<std::string, std::string>& names) const
    {
        return std::hash<std::string>{}(names.first) + std::hash<std::string>{}(names.second) * 37;
    }
};
class TransportCatalogue
{
public:
	void AddStop(const std::string& name, const polar_coordinates::Coordinates& coords, std::vector<std::pair<std::string, double>> distances_to);
	void AddRoute(const std::string& name, const std::vector<std::string>& stops_names, bool circular);

	Stop GetStop(const std::string& name);
	Route GetRoute(const std::string& name);

	double GetDistanceBetweenStops(const std::string& name_a, const std::string& name_b) const;

	double CalculateRouteLength(const std::string& name) const;
	double CalculateRouteLength_RAW(const std::string& name) const;
private:
	std::unordered_map<std::string, Stop> all_stops;
	std::unordered_map<std::string, Route> all_routes;
	std::unordered_map<std::pair<std::string, std::string>, double, StopPairHasher> distances;
};