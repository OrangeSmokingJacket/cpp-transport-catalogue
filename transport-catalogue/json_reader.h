#pragma once

#include "json.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <algorithm>
#include <sstream>


void ParseInputRequest(const json::Node& request, TransportCatalogue& catalogue);
void ParseOutputRequest(const json::Node& request, TransportCatalogue& catalogue, renderer::MapRenderer& map_renderer, json::Array& output);
void ParseRenderSetting(const json::Dict& render_settings, renderer::MapRenderer& map_renderer);

svg::Color ParseColor(const json::Node& node);
json::Dict ParseJSON(std::istream& input);