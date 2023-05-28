#include "request_handler.h"

void RequestHandler::ParseInput(std::istream& input)
{
    json::Dict requests = ParseJSON(input);

    json::Array input_requests = requests.at("base_requests").AsArray();
    for (const json::Node& request : input_requests)
    {
        ParseInputRequest(request);
    }

	json::Dict render_settings = requests.at("render_settings").AsMap();
	ParseRenderSetting(render_settings);

    json::Array output_requests = requests.at("stat_requests").AsArray();
    for (const json::Node& request : output_requests)
    {
        ParseOutputRequest(request);
    }
}
json::Document RequestHandler::ReturnDocument()
{
    json::Document result(output);
    return result;
}

void RequestHandler::ParseInputRequest(const json::Node& request)
{
	json::Dict specifics = request.AsMap();
	std::string type = specifics.at("type").AsString();
	std::string name = specifics.at("name").AsString();

	if (type == "Stop")
	{
		polar_coordinates::Coordinates coords(0.0, 0.0);
		std::vector<std::pair<std::string, double>> distances;

		if (specifics.contains("latitude"))
			coords.lat = specifics.at("latitude").AsDouble();
		if (specifics.contains("longitude"))
			coords.lng = specifics.at("longitude").AsDouble();
		if (specifics.contains("road_distances"))
		{
			for (const auto& [stop, dst] : specifics.at("road_distances").AsMap())
			{
				distances.push_back({ stop, dst.AsDouble() });
			}
		}
		catalogue.AddStop(name, coords);
		if (distances.size() != 0)
			catalogue.AddStopsDistances(name, distances);

		return;
	}
	if (type == "Bus")
	{
		bool is_roundtrip = false;
		std::vector<std::string> stops;

		if (specifics.contains("is_roundtrip"))
			is_roundtrip = specifics.at("is_roundtrip").AsBool();
		if (specifics.contains("stops"))
		{
			stops.reserve(specifics.at("stops").AsArray().size());
			for (const json::Node& stop : specifics.at("stops").AsArray())
			{
				stops.push_back(stop.AsString());
			}
		}
		catalogue.AddRoute(name, stops, is_roundtrip);

		return;
	}

	throw std::logic_error("\"type\" is not correct.");
}
void RequestHandler::ParseOutputRequest(const json::Node& request)
{
	json::Dict result;
	json::Dict specifics = request.AsMap();
	int id = specifics.at("id").AsInt();
	std::string type = specifics.at("type").AsString();

	result.insert({ "request_id", id });
	if (type == "Stop")
	{
		std::string name = specifics.at("name").AsString();
		std::optional<Stop> stop = catalogue.GetStop(name);
		if (stop)
		{
			json::Array routes_arr;
			std::set<std::string> routes;
			for (Route* route : stop.value().GetRoutes())
			{
				routes.insert(route->GetName());
			}
			for (const std::string& route : routes)
			{
				routes_arr.push_back(route);
			}
			result.insert({ "buses",  routes_arr });
		}
		else
		{
			using namespace std::literals;
			json::Node error_node = { "not found"s };		 //for some reson, without string literal c++ thinks it is a boolean...
			result.insert({ "error_message", error_node });
		}

		output.emplace_back(result);
	}
	if (type == "Bus")
	{
		std::string name = specifics.at("name").AsString();
		std::optional<Route> route = catalogue.GetRoute(name);
		if (route)
		{
			double route_length = catalogue.CalculateRouteLength(name);
			result.insert({ "curvature",  route_length / catalogue.CalculateRouteLength_RAW(name) });
			result.insert({ "route_length", route_length });
			result.insert({ "stop_count", route.value().GetStopsCount() });
			result.insert({ "unique_stop_count", route.value().GetRoutesUniqueStops() });
		}
		else
		{
			using namespace std::literals;
			json::Node error_node = { "not found"s };		 //for some reson, without string literal c++ thinks it is a boolean...
			result.insert({ "error_message", error_node });
		}

		output.emplace_back(result);
	}
	if (type == "Map")
	{
		std::ostringstream s;
		map_renderer.CreateCanvas(catalogue.GetAllRoutes()).Render(s);
		result.insert({ "map", s.str() });

		output.emplace_back(result);
	}
}
void RequestHandler::ParseRenderSetting(const json::Dict& render_settings)
{
	if (render_settings.contains("width"))
		map_renderer.SetWidth(render_settings.at("width").AsDouble());
	if (render_settings.contains("height"))
		map_renderer.SetHeight(render_settings.at("height").AsDouble());
	if (render_settings.contains("padding"))
		map_renderer.SetPadding(render_settings.at("padding").AsDouble());
	if (render_settings.contains("line_width"))
		map_renderer.SetLineWidth(render_settings.at("line_width").AsDouble());
	if (render_settings.contains("stop_radius"))
		map_renderer.SetStopRadius(render_settings.at("stop_radius").AsDouble());
	if (render_settings.contains("stop_label_font_size"))
		map_renderer.SetStopLabelFontSize(render_settings.at("stop_label_font_size").AsInt());
	if (render_settings.contains("stop_label_offset"))
		map_renderer.SetStopLabelOffset(render_settings.at("stop_label_offset").AsArray().at(0).AsDouble(), render_settings.at("stop_label_offset").AsArray().at(1).AsDouble());
	if (render_settings.contains("bus_label_font_size"))
		map_renderer.SetBusLabelFontSize(render_settings.at("bus_label_font_size").AsInt());
	if (render_settings.contains("bus_label_offset"))
		map_renderer.SetBusLabelOffset(render_settings.at("bus_label_offset").AsArray().at(0).AsDouble(), render_settings.at("bus_label_offset").AsArray().at(1).AsDouble());
	if (render_settings.contains("underlayer_color"))
		map_renderer.SetUnderlayerColor(std::move(ParseColor(render_settings.at("underlayer_color"))));
	if (render_settings.contains("underlayer_width"))
		map_renderer.SetUnderlayerWidth(render_settings.at("underlayer_width").AsDouble());
	if (render_settings.contains("color_palette"))
	{
		for (const json::Node& n : render_settings.at("color_palette").AsArray())
		{
			map_renderer.AddToColorPallete(std::move(ParseColor(n)));
		}
	}
}

svg::Color RequestHandler::ParseColor(const json::Node& node)
{
	if (node.IsString())
		return node.AsString();

	size_t size = node.AsArray().size();
	if (size == 3)
	{
		svg::Rgb rgb(static_cast<uint8_t>(node.AsArray().at(0).AsInt()), static_cast<uint8_t>(node.AsArray().at(1).AsInt()), static_cast<uint8_t>(node.AsArray().at(2).AsInt()));
		return rgb;
	}
	else
	{
		svg::Rgba rgba(static_cast<uint8_t>(node.AsArray().at(0).AsInt()), static_cast<uint8_t>(node.AsArray().at(1).AsInt()), static_cast<uint8_t>(node.AsArray().at(2).AsInt()), node.AsArray().at(3).AsDouble());
		return rgba;
	}
}
json::Dict RequestHandler::ParseJSON(std::istream& input)
{
	json::Document doc = json::Load(input);
	return doc.GetRoot().AsMap();
}