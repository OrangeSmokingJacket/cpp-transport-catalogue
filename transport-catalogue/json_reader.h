#pragma once

#include "json.h"
#include "svg.h"
#include "request_handler.h"

#include <algorithm>
#include <sstream>


void ParseInput(std::istream& input, RequestHandler& request_handler);

void ParseInputRequest(const json::Node& request, TransportCatalogue& catalogue);
void ParseOutputRequest(const json::Node& request, RequestHandler& request_handler);
void ParseRenderSetting(const json::Dict& render_settings, renderer::MapRenderer& map_renderer);

svg::Color ParseColor(const json::Node& node);
json::Dict ParseJSON(std::istream& input);