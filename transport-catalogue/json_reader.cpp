#include "json_reader.h"

using namespace std::literals;

void ParseInput(std::istream& input, RequestHandler& request_handler, std::string_view mode)
{
	json::Dict requests = ParseJSON(input);

	if (mode == "make_base"sv)
	{
		json::Array input_requests = requests.at("base_requests").AsArray();
		for (const json::Node& request : input_requests)
		{
			ParseInputRequest(request, request_handler.GetCatalogueRef());
		}

		json::Dict routing_settings = requests.at("routing_settings").AsDict();
		ParseRoutingSettings(routing_settings, request_handler);

		json::Dict render_settings = requests.at("render_settings").AsDict();
		ParseRenderSettings(render_settings, request_handler.GetRendererRef());

		json::Dict serialization_settings = requests.at("serialization_settings").AsDict();
		ParseSerializationSettings(serialization_settings, request_handler, mode);
	}
	else if (mode == "process_requests"sv)
	{
		json::Dict serialization_settings = requests.at("serialization_settings").AsDict();
		ParseSerializationSettings(serialization_settings, request_handler, mode);

		json::Array output_requests = requests.at("stat_requests").AsArray();
		for (const json::Node& request : output_requests)
		{
			ParseOutputRequest(request, request_handler);
		}
	}

}

void ParseInputRequest(const json::Node& request, TransportCatalogue& catalogue)
{
	json::Dict specifics = request.AsDict();
	std::string type = specifics.at("type").AsString();
	std::string name = specifics.at("name").AsString();

	if (type == "Stop")
	{
		polar_coordinates::Coordinates coords(0.0, 0.0);
		std::vector<std::pair<std::string, double>> distances;

		if (specifics.count("latitude"))
			coords.lat = specifics.at("latitude").AsDouble();
		if (specifics.count("longitude"))
			coords.lng = specifics.at("longitude").AsDouble();
		if (specifics.count("road_distances"))
		{
			for (const auto& [stop, dst] : specifics.at("road_distances").AsDict())
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

		if (specifics.count("is_roundtrip"))
			is_roundtrip = specifics.at("is_roundtrip").AsBool();
		if (specifics.count("stops"))
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
void ParseRoutingSettings(const json::Dict& routing_settings, RequestHandler& request_handler)
{
	if (routing_settings.count("bus_wait_time") == 0 || routing_settings.count("bus_velocity") == 0)
		throw std::logic_error("incomplete route_settings");

	request_handler.SetBusSpeed(routing_settings.at("bus_velocity").AsDouble());
	request_handler.SetWaitingTime(routing_settings.at("bus_wait_time").AsInt());
}
void ParseOutputRequest(const json::Node& request, RequestHandler& request_handler)
{
	json::Builder result;
	result.StartDict();
	json::Dict specifics = request.AsDict();
	int id = specifics.at("id").AsInt();
	std::string type = specifics.at("type").AsString();

	result.Key("request_id").Value(id);
	if (type == "Stop")
	{
		std::string name = specifics.at("name").AsString();
		std::optional<StopStat> stat = request_handler.GetStopStat(name);
		if (stat)
			result.Key("buses").Value(stat.value().routes_arr);
		else
			result.Key("error_message").Value("not found");

	}
	else if (type == "Bus")
	{
		std::string name = specifics.at("name").AsString();
		std::optional<BusStat> stat = request_handler.GetBusStat(name);
		if (stat)
		{
			result.Key("curvature").Value(stat.value().curvature);
			result.Key("route_length").Value(stat.value().route_length);
			result.Key("stop_count").Value(stat.value().stop_count);
			result.Key("unique_stop_count").Value(stat.value().unique_stop_count);
		}
		else
		{
			result.Key("error_message").Value("not found");
		}
	}
	else if (type == "Route")
	{
		std::string from = specifics.at("from").AsString();
		std::string to = specifics.at("to").AsString();
		std::optional<RouteStat> stat = request_handler.GetRouteStat(from, to);
		if (stat)
		{
			result.Key("total_time").Value(stat.value().info.weight);
			result.Key("items").StartArray();
			for (const graph::EdgeId id : stat.value().info.edges)
			{
				result.StartDict();
				result.Key("type").Value("Wait");
				result.Key("stop_name").Value(request_handler.GetRouterRef().GetStopName(request_handler.GetRouterRef().GetEdge(id).from));
				result.Key("time").Value(request_handler.GetWaitingTime());
				result.EndDict();

				result.StartDict();
				result.Key("type").Value("Bus");
				std::pair<std::string, size_t> edge_data = request_handler.GetRouterRef().GetEdgeData(id);
				result.Key("bus").Value(edge_data.first);
				result.Key("span_count").Value(static_cast<int>(edge_data.second));
				result.Key("time").Value(request_handler.GetRouterRef().GetEdge(id).weight - request_handler.GetWaitingTime());
				result.EndDict();
			}
			result.EndArray();
		}
		else
		{
			result.Key("error_message").Value("not found");
		}
	}
	else if (type == "Map")
	{
		std::ostringstream s;
		request_handler.RenderMap().Render(s);
		result.Key("map").Value(s.str());
	}
	result.EndDict();
	request_handler.AddToOutput(std::move(result.Build()));
	return;
}
void ParseRenderSettings(const json::Dict& render_settings, renderer::MapRenderer& map_renderer)
{
	if (render_settings.count("width"))
		map_renderer.SetWidth(render_settings.at("width").AsDouble());
	if (render_settings.count("height"))
		map_renderer.SetHeight(render_settings.at("height").AsDouble());
	if (render_settings.count("padding"))
		map_renderer.SetPadding(render_settings.at("padding").AsDouble());
	if (render_settings.count("line_width"))
		map_renderer.SetLineWidth(render_settings.at("line_width").AsDouble());
	if (render_settings.count("stop_radius"))
		map_renderer.SetStopRadius(render_settings.at("stop_radius").AsDouble());
	if (render_settings.count("stop_label_font_size"))
		map_renderer.SetStopLabelFontSize(render_settings.at("stop_label_font_size").AsInt());
	if (render_settings.count("stop_label_offset"))
		map_renderer.SetStopLabelOffset(render_settings.at("stop_label_offset").AsArray().at(0).AsDouble(), render_settings.at("stop_label_offset").AsArray().at(1).AsDouble());
	if (render_settings.count("bus_label_font_size"))
		map_renderer.SetBusLabelFontSize(render_settings.at("bus_label_font_size").AsInt());
	if (render_settings.count("bus_label_offset"))
		map_renderer.SetBusLabelOffset(render_settings.at("bus_label_offset").AsArray().at(0).AsDouble(), render_settings.at("bus_label_offset").AsArray().at(1).AsDouble());
	if (render_settings.count("underlayer_color"))
		map_renderer.SetUnderlayerColor(std::move(ParseColor(render_settings.at("underlayer_color"))));
	if (render_settings.count("underlayer_width"))
		map_renderer.SetUnderlayerWidth(render_settings.at("underlayer_width").AsDouble());
	if (render_settings.count("color_palette"))
	{
		for (const json::Node& n : render_settings.at("color_palette").AsArray())
		{
			map_renderer.AddToColorPallete(std::move(ParseColor(n)));
		}
	}
}
void ParseSerializationSettings(const json::Dict& serialization_settings, RequestHandler& request_handler, std::string_view mode)
{
	Serializator serializator(request_handler, serialization_settings.at("file").AsString());

	if (mode == "make_base"sv)
		serializator.Save();
	else if (mode == "process_requests"sv)
		serializator.Load();
}

svg::Color ParseColor(const json::Node& node)
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
json::Dict ParseJSON(std::istream& input)
{
	json::Document doc = json::Load(input);
	return doc.GetRoot().AsDict();
}