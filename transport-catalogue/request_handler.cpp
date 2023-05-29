#include "request_handler.h"

void RequestHandler::ParseInput(std::istream& input)
{
    json::Dict requests = ParseJSON(input);

    json::Array input_requests = requests.at("base_requests").AsArray();
    for (const json::Node& request : input_requests)
    {
        ParseInputRequest(request, catalogue);
    }

	json::Dict render_settings = requests.at("render_settings").AsMap();
	ParseRenderSetting(render_settings, map_renderer);

    json::Array output_requests = requests.at("stat_requests").AsArray();
    for (const json::Node& request : output_requests)
    {
        ParseOutputRequest(request, catalogue, map_renderer, output);
    }
}
json::Document RequestHandler::ReturnDocument()
{
    json::Document result(output);
    return result;
}
