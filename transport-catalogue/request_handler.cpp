#include "request_handler.h"


void RequestHandler::SetBusSpeed(double speed)
{
	bus_speed = speed / 60 * 1000;
}
void RequestHandler::SetWaitingTime(int time)
{
	waiting_time = time;
}
double RequestHandler::GetBusSpeed()
{
	return bus_speed;
}
int RequestHandler::GetWaitingTime()
{
	return waiting_time;
}

std::optional<BusStat> RequestHandler::GetBusStat(const std::string& name) const
{
    BusStat result;
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
		for (const Route* route : stop.value().GetRoutes())
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
std::optional<RouteStat> RequestHandler::GetRouteStat(const std::string& from, const std::string& to)
{
	if (!router)
		router = std::make_unique<TransportRouter>(catalogue, bus_speed, waiting_time);

	std::optional<graph::Router<double>::RouteInfo> info = router.get()->FindPath(from, to);

	if (info.has_value())
		return RouteStat{ info.value() };
	else
		return std::nullopt;
}

svg::Document RequestHandler::RenderMap() const
{
	return map_renderer.CreateCanvas(catalogue.GetAllRoutes());
}

void RequestHandler::AddToOutput(json::Node&& result)
{
	output.Value(result);
}
json::Document RequestHandler::ReturnDocument()
{
    json::Document result(output.EndArray().Build());
    return result;
}

TransportCatalogue& RequestHandler::GetCatalogueRef()
{
	return catalogue;
}
TransportRouter& RequestHandler::GetRouterRef()
{
	return *router.get();
}
renderer::MapRenderer& RequestHandler::GetRendererRef()
{
	return map_renderer;
}

catalogue::TransportCatalogue RequestHandler::SerializeTransportCatalogue() const
{
	return catalogue.Serialize();
}
rend::MapRenderer RequestHandler::SerializeMapRenderer() const
{
	return map_renderer.Serialize();
}
router::RouterSettings RequestHandler::SerializeRouterSettings() const
{
	router::RouterSettings settings;

	settings.set_bus_speed(bus_speed);
	settings.set_stop_waiting_time(waiting_time);

	return settings;
}

void RequestHandler::DeserializeTransportCatalogue(catalogue::TransportCatalogue t_cat)
{
	catalogue.Deserialize(t_cat);
}
void RequestHandler::DeserializeMapRenderer(rend::MapRenderer map_rend)
{
	map_renderer.Deserialize(map_rend);
}
void RequestHandler::DeserializeRouterSettings(router::RouterSettings settings)
{
	bus_speed = settings.bus_speed();
	waiting_time = settings.stop_waiting_time();
}
