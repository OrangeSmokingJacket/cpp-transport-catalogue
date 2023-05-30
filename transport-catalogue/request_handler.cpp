#include "request_handler.h"


std::optional<RouteStat> RequestHandler::GetBusStat(const std::string& name) const
{
    RouteStat result;
	std::optional<Route> route = catalogue.GetRoute(name);
	if (route)
	{
		result.route_length = catalogue.CalculateRouteLength(name);
		result.curvature = result.route_length / catalogue.CalculateRouteLength_RAW(name);
		result.stop_count = route.value().GetStopsCount();
		result.unique_stop_count = route.value().GetRoutesUniqueStops();

		return result;
	}
	else
	{
		return {};
	}
}
std::optional<StopStat> RequestHandler::GetStopStat(const std::string& name) const
{
	StopStat result;
	std::optional<Stop> stop = catalogue.GetStop(name);
	if (stop)
	{
		std::set<std::string> routes;
		for (Route* route : stop.value().GetRoutes())
		{
			routes.insert(route->GetName());
		}
		for (const std::string& route : routes)
		{
			result.routes_arr.push_back(route);
		}
		return result;
	}
	else
	{
		return {};
	}
}

svg::Document RequestHandler::RenderMap() const
{
	return map_renderer.CreateCanvas(catalogue.GetAllRoutes());
}

void RequestHandler::AddToOutput(json::Dict&& result)
{
	output.emplace_back(result);
}
json::Document RequestHandler::ReturnDocument()
{
    json::Document result(output);
    return result;
}

TransportCatalogue& RequestHandler::GetCatalogueRef()
{
	return catalogue;
}
renderer::MapRenderer& RequestHandler::GetRendererRef()
{
	return map_renderer;
}
