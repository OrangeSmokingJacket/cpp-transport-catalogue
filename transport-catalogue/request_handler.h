#pragma once

#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include "transport_router.h"

struct BusStat
{
    double route_length;
    double curvature;
    int stop_count;
    int unique_stop_count;
};
struct StopStat
{
    json::Array routes_arr;
};
struct RouteStat
{
    graph::Router<double>::RouteInfo info;
};
 class RequestHandler
 {
 public:
     RequestHandler(TransportCatalogue& tc, renderer::MapRenderer& renderer_) : catalogue(tc), map_renderer(renderer_)
     {
         output.StartArray();
     }

     void SetBusSpeed(double speed);
     void SetWaitingTime(int time);
     double GetBusSpeed();
     int GetWaitingTime();

     std::optional<BusStat> GetBusStat(const std::string& route_name) const;
     std::optional<StopStat> GetStopStat(const std::string& stop_name) const;
     std::optional<RouteStat> GetRouteStat(const std::string& from, const std::string& to);

     svg::Document RenderMap() const;

     void AddToOutput(json::Node&& result);
	 json::Document ReturnDocument();

     TransportCatalogue& GetCatalogueRef();
     TransportRouter& GetRouterRef();
     renderer::MapRenderer& GetRendererRef();

     catalogue::TransportCatalogue SerializeTransportCatalogue() const;
     rend::MapRenderer SerializeMapRenderer() const;
     router::RouterSettings SerializeRouterSettings() const;

     void DeserializeTransportCatalogue(catalogue::TransportCatalogue t_cat);
     void DeserializeMapRenderer(rend::MapRenderer map_rend);
     void DeserializeRouterSettings(router::RouterSettings settings);

 private:

     TransportCatalogue& catalogue;
     std::unique_ptr<TransportRouter> router = nullptr;
     json::Builder output;
     renderer::MapRenderer& map_renderer;

     double bus_speed;
     int waiting_time;
 };