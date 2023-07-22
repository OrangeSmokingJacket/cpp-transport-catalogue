#include "serialization.h"

void Serializator::Save()
{
    std::ofstream ofs(path, std::ios::binary);

    catalogue::WholeMessage whole_message;

    *whole_message.mutable_catalogue() = std::move(SerializeTransportCatalogue());
    *whole_message.mutable_renderer() = std::move(SerializeMapRenderer());
    *whole_message.mutable_router_settings() = std::move(SerializeTransportRouter());

    whole_message.SerializeToOstream(&ofs);
}
void Serializator::Load()
{
    std::ifstream ifs(path, std::ios::binary);
    
    catalogue::WholeMessage whole_message;
	if (!whole_message.ParseFromIstream(&ifs))
	{
        throw std::runtime_error("Couldn't parse input");
	}

    DeserializeTransportCatalogue(whole_message.catalogue());
    DeserializeMapRenderer(whole_message.renderer());
	DeserializeTransportRouter(whole_message.router_settings());
}

rend::Color SVG_2_Rend(const svg::Color& color)
{
	rend::Color c;
	if (std::holds_alternative<std::string>(color))
	{
		rend::Color_String color_string;
		color_string.set_color(std::get<std::string>(color));
		*c.mutable_color_string() = std::move(color_string);
	}
	else if (std::holds_alternative<svg::Rgb>(color))
	{
		rend::Color_RGB rgb;

		rgb.set_r(std::get<svg::Rgb>(color).red);
		rgb.set_g(std::get<svg::Rgb>(color).green);
		rgb.set_b(std::get<svg::Rgb>(color).blue);

		*c.mutable_color_rgb() = std::move(rgb);
	}
	else if (std::holds_alternative<svg::Rgba>(color))
	{
		rend::Color_RGBA rgba;

		rgba.set_r(std::get<svg::Rgba>(color).red);
		rgba.set_g(std::get<svg::Rgba>(color).green);
		rgba.set_b(std::get<svg::Rgba>(color).blue);
		rgba.set_opacity(std::get<svg::Rgba>(color).opacity);

		*c.mutable_color_rgba() = std::move(rgba);
	}
	return c;
}
rend::Point SVG_2_Rend(const svg::Point& point)
{
	rend::Point p;
	p.set_x(point.x);
	p.set_y(point.y);
	return p;
}
svg::Color Rend_2_SVG(const rend::Color& color)
{
	if (color.has_color_rgba())
	{
		svg::Rgba rgba;

		rgba.red = color.color_rgba().r();
		rgba.green = color.color_rgba().g();
		rgba.blue = color.color_rgba().b();
		rgba.opacity = color.color_rgba().opacity();

		return rgba;
	}
	else if (color.has_color_rgb())
	{
		svg::Rgb rgb;

		rgb.red = color.color_rgb().r();
		rgb.green = color.color_rgb().g();
		rgb.blue = color.color_rgb().b();

		return rgb;
	}
	else if (color.has_color_string())
	{
		return { color.color_string().color() };
	}
	else
		return {};
}
svg::Point Rend_2_SVG(const rend::Point& point)
{
	return { point.x(), point.y() };
}

catalogue::TransportCatalogue Serializator::SerializeTransportCatalogue() const
{
	catalogue::TransportCatalogue t_cat;

	for (const auto& [name, stop] : handler.GetCatalogueRef().GetStopsRef())
	{
		catalogue::Stop s;
		s.set_name(name.data(), name.length());

		catalogue::Coordinates coords;
		coords.set_lat(stop.GetCoordinates().lat);
		coords.set_lng(stop.GetCoordinates().lng);
		*s.mutable_coords() = std::move(coords);

		(*t_cat.mutable_all_stops()).Add(std::move(s));
	}

	for (const auto& [name, route] : handler.GetCatalogueRef().GetRoutesRef())
	{
		catalogue::Route r;
		r.set_name(name.data(), name.length());

		for (const Stop* stop : route.GetStops())
		{
			r.add_stop_names(stop->GetName().data(), stop->GetName().length());
		}
		r.set_is_circular(route.GetRouteType() == Route::RouteType::Curcular);
		(*t_cat.mutable_all_routes()).Add(std::move(r));
	}

	for (const auto& [stop_pair, dis] : handler.GetCatalogueRef().GetDistancesRef())
	{
		catalogue::Distance d;
		d.set_from(stop_pair.first.data(), stop_pair.first.length());
		d.set_to(stop_pair.second.data(), stop_pair.second.length());
		d.set_distance(dis);
		(*t_cat.mutable_distances()).Add(std::move(d));
	}

	return t_cat;
}
rend::MapRenderer Serializator::SerializeMapRenderer() const
{
	renderer::MapRendererSettings settings = handler.GetRendererRef().GetSettings();
	rend::MapRenderer map_renderer;

	map_renderer.set_width(settings.width);
	map_renderer.set_height(settings.height);
	map_renderer.set_padding(settings.padding);
	map_renderer.set_line_width(settings.line_width);
	map_renderer.set_stop_radius(settings.stop_radius);
	map_renderer.set_stop_label_font_size(settings.stop_label_font_size);
	map_renderer.set_bus_label_font_size(settings.bus_label_font_size);
	map_renderer.set_underlayer_width(settings.underlayer_width);

	for (const svg::Color& color : settings.color_pallete)
	{
		(*map_renderer.mutable_pallete()).Add(std::move(SVG_2_Rend(color)));
	}

	*map_renderer.mutable_stop_label_offset() = std::move(SVG_2_Rend(settings.stop_label_offset));
	*map_renderer.mutable_bus_label_offset() = std::move(SVG_2_Rend(settings.bus_label_offset));
	*map_renderer.mutable_underlayer_color() = std::move(SVG_2_Rend(settings.underlayer_color));

	return map_renderer;
}
router::RouterSettings Serializator::SerializeTransportRouter() const
{
	router::RouterSettings settings;

	settings.set_bus_speed(handler.GetBusSpeed());
	settings.set_stop_waiting_time(handler.GetWaitingTime());

	return settings;
}

void Serializator::DeserializeTransportCatalogue(catalogue::TransportCatalogue t_cat) const
{
	for (catalogue::Route& r : *t_cat.mutable_all_routes())
	{
		std::vector<std::string> stop_names;
		stop_names.reserve(r.stop_names_size());
		for (const std::string& stop : *r.mutable_stop_names())
		{
			stop_names.push_back(stop);
		}

		handler.GetCatalogueRef().AddRoute(r.name(), stop_names, r.is_circular());
	}

	for (catalogue::Stop& s : *t_cat.mutable_all_stops())
	{
		polar_coordinates::Coordinates coord;
		coord.lat = s.coords().lat();
		coord.lng = s.coords().lng();

		handler.GetCatalogueRef().AddStop(s.name(), coord);
	}

	for (const catalogue::Distance& d : *t_cat.mutable_distances())
	{
		handler.GetCatalogueRef().AddStopsDistances(d.from(), d.to(), d.distance());
	}
}
void Serializator::DeserializeMapRenderer(rend::MapRenderer map_renderer) const
{
	handler.GetRendererRef().SetWidth(map_renderer.width());
	handler.GetRendererRef().SetHeight(map_renderer.height());
	handler.GetRendererRef().SetPadding(map_renderer.padding());
	handler.GetRendererRef().SetLineWidth(map_renderer.line_width());
	handler.GetRendererRef().SetStopRadius(map_renderer.stop_radius());
	handler.GetRendererRef().SetStopLabelFontSize(map_renderer.stop_label_font_size());
	handler.GetRendererRef().SetBusLabelFontSize(map_renderer.bus_label_font_size());
	handler.GetRendererRef().SetUnderlayerWidth(map_renderer.underlayer_width());

	for (rend::Color c : map_renderer.pallete())
	{
		handler.GetRendererRef().AddToColorPallete(std::move(Rend_2_SVG(c)));
	}

	handler.GetRendererRef().SetStopLabelOffset(Rend_2_SVG(map_renderer.stop_label_offset()));
	handler.GetRendererRef().SetBusLabelOffset(Rend_2_SVG(map_renderer.bus_label_offset()));
	handler.GetRendererRef().SetUnderlayerColor(Rend_2_SVG(map_renderer.underlayer_color()));
}
void Serializator::DeserializeTransportRouter(router::RouterSettings settings) const
{
	handler.SetBusSpeed(settings.bus_speed());
	handler.SetWaitingTime(settings.stop_waiting_time());
}