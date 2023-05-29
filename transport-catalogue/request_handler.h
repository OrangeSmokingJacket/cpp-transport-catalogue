#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"


 class RequestHandler
 {
 public:
     RequestHandler(TransportCatalogue& tc, renderer::MapRenderer& renderer_) : catalogue(tc), map_renderer(renderer_) {}

	 void ParseInput(std::istream& input);
	 json::Document ReturnDocument();

 private:

     TransportCatalogue& catalogue;
	 json::Array output;
     renderer::MapRenderer& map_renderer;
 };