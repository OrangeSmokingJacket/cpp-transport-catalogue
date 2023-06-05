#pragma once

#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

struct RouteStat
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
 class RequestHandler
 {
 public:
     RequestHandler(TransportCatalogue& tc, renderer::MapRenderer& renderer_) : catalogue(tc), map_renderer(renderer_)
     {
         output.StartArray();
     }

     std::optional<RouteStat> GetBusStat(const std::string& route_name) const;
     std::optional<StopStat> GetStopStat(const std::string& stop_name) const;

     svg::Document RenderMap() const;

     void AddToOutput(json::Node&& result);
	 json::Document ReturnDocument();

     TransportCatalogue& GetCatalogueRef();
     renderer::MapRenderer& GetRendererRef();

 private:

     TransportCatalogue& catalogue;
     json::Builder output;
     renderer::MapRenderer& map_renderer;
 };