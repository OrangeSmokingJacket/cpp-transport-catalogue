#pragma once

#include "request_handler.h"

#include <filesystem>
#include <fstream>

class Serializator
{
public:
    Serializator(RequestHandler& handler, const std::string& path) : handler(handler), path(path) { }
    void Save();
    void Load();

private:
    catalogue::TransportCatalogue SerializeTransportCatalogue() const;
    rend::MapRenderer SerializeMapRenderer() const;
    router::RouterSettings SerializeTransportRouter() const;

    void DeserializeTransportCatalogue(catalogue::TransportCatalogue t_cat) const;
    void DeserializeMapRenderer(rend::MapRenderer map_renderer) const;
    void DeserializeTransportRouter(router::RouterSettings settings) const;

    std::filesystem::path path;
    RequestHandler& handler;
};