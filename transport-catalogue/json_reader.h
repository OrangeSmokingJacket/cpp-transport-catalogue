#pragma once

#include "svg.h"
#include "request_handler.h"
#include "serialization.h"

#include <algorithm>
#include <sstream>


void ParseInput(std::istream& input, RequestHandler& request_handler, std::string_view mode);

void ParseInputRequest(const json::Node& request, TransportCatalogue& catalogue);
void ParseRoutingSettings(const json::Dict& routing_settings, RequestHandler& request_handler);
void ParseOutputRequest(const json::Node& request, RequestHandler& request_handler);
void ParseRenderSettings(const json::Dict& render_settings, renderer::MapRenderer& map_renderer);
void ParseSerializationSettings(const json::Dict& serialization_settings, RequestHandler& handler, std::string_view mode);

svg::Color ParseColor(const json::Node& node);
json::Dict ParseJSON(std::istream& input);