#pragma once

#include "json.h"
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
     RequestHandler(TransportCatalogue& tc, renderer::MapRenderer& renderer_) : catalogue(tc), map_renderer(renderer_) {}

     std::optional<RouteStat> GetBusStat(const std::string& route_name) const;
     std::optional<StopStat> GetStopStat(const std::string& stop_name) const;

     svg::Document RenderMap() const;

     void AddToOutput(json::Dict&& result);
	 json::Document ReturnDocument();

     TransportCatalogue& GetCatalogueRef();
     renderer::MapRenderer& GetRendererRef();

 private:

     TransportCatalogue& catalogue;
	 json::Array output;
     renderer::MapRenderer& map_renderer;
 };
 /*
class RequestHandler {
public:
    // MapRenderer понадобится в следующей части итогового проекта
    RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);

    // Возвращает информацию о маршруте (запрос Bus)
    std::optional<BusStat> GetBusStat(const std::string_view& bus_name) const;

    // Возвращает маршруты, проходящие через
    const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;

    // Этот метод будет нужен в следующей части итогового проекта
    svg::Document RenderMap() const;

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& db_;
    const renderer::MapRenderer& renderer_;
};
*/