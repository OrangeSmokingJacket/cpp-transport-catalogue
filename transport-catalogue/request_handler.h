#pragma once

#include "transport_catalogue.h"
#include "json.h"
#include "map_renderer.h"

#include <algorithm>
#include <sstream>


 class RequestHandler
 {
 public:
     RequestHandler(TransportCatalogue& tc, renderer::MapRenderer& renderer_) : catalogue(tc), map_renderer(renderer_) {}

	 void ParseInput(std::istream& input);
	 json::Document ReturnDocument();

 private:

     void ParseInputRequest(const json::Node& request);
	 void ParseOutputRequest(const json::Node& request);
	 void ParseRenderSetting(const json::Dict& render_settings);

     svg::Color ParseColor(const json::Node& node);
	 json::Dict ParseJSON(std::istream& input);

     TransportCatalogue& catalogue;
	 json::Array output;
     renderer::MapRenderer& map_renderer;
 };