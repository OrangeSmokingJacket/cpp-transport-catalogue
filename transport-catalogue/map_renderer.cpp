#include "map_renderer.h"

#include <limits>
#include <algorithm>

namespace renderer
{

	inline const double EPSILON = 1e-6;
	bool IsZero(double value)
	{
		return std::abs(value) < EPSILON;
	}

	class SphereProjector
	{
	public:
		template <typename PointInputIt>
		SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width, double max_height, double padding_) : padding(padding_)
		{
			if (points_begin == points_end)
				return;

			const auto [left_it, right_it] = std::minmax_element(points_begin, points_end,
				[](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });

			min_lon = left_it->lng;
			const double max_lon = right_it->lng;

			const auto [bottom_it, top_it] = std::minmax_element(points_begin, points_end,
				[](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });

			const double min_lat = bottom_it->lat;
			max_lat = top_it->lat;

			std::optional<double> width_zoom;
			if (!IsZero(max_lon - min_lon))
				width_zoom = (max_width - 2 * padding) / (max_lon - min_lon);

			std::optional<double> height_zoom;
			if (!IsZero(max_lat - min_lat))
				height_zoom = (max_height - 2 * padding) / (max_lat - min_lat);

			if (width_zoom && height_zoom)
				zoom_coeff = std::min(*width_zoom, *height_zoom);
			else if (width_zoom)
				zoom_coeff = *width_zoom;
			else if (height_zoom)
				zoom_coeff = *height_zoom;
		}

		svg::Point operator()(polar_coordinates::Coordinates coords) const
		{
			return
			{
				(coords.lng - min_lon) * zoom_coeff + padding,
				(max_lat - coords.lat) * zoom_coeff + padding
			};
		}

	private:
		double padding;
		double min_lon = 0;
		double max_lat = 0;
		double zoom_coeff = 0;
	};

	void MapRenderer::SetWidth(double w)
	{
		width = w;
	}
	void MapRenderer::SetHeight(double h)
	{
		height = h;
	}
	void MapRenderer::SetPadding(double p)
	{
		padding = p;
	}
	void MapRenderer::SetLineWidth(double lw)
	{
		line_width = lw;
	}
	void MapRenderer::SetStopRadius(double r)
	{
		stop_radius = r;
	}
	void MapRenderer::SetStopLabelFontSize(int size)
	{
		stop_label_font_size = size;
	}
	void MapRenderer::SetStopLabelOffset(double dx, double dy)
	{
		stop_label_offset.x = dx;
		stop_label_offset.y = dy;
	}
	void MapRenderer::SetBusLabelFontSize(int size)
	{
		bus_label_font_size = size;
	}
	void MapRenderer::SetBusLabelOffset(double dx, double dy)
	{
		bus_label_offset.x = dx;
		bus_label_offset.y = dy;
	}
	void MapRenderer::SetUnderlayerColor(svg::Color color)
	{
		underlayer_color = std::move(color);
	}
	void MapRenderer::SetUnderlayerWidth(double ulw)
	{
		underlayer_width = ulw;
	}
	void MapRenderer::AddToColorPallete(svg::Color color)
	{
		color_pallete.push_back(std::move(color));
	}

	svg::Document MapRenderer::CreateCanvas(std::vector<Route*> routes)
	{
		std::vector<polar_coordinates::Coordinates> coords;
		std::vector<Stop*> unique_stops; // will be needed later
		for (Route* route : routes)
		{
			for (Stop* stop : route->GetStops())
			{
				coords.push_back(stop->GetCoordinates());
				unique_stops.push_back(stop);
			}
		}
		std::sort(coords.begin(), coords.end(), [](const polar_coordinates::Coordinates& lhs, const polar_coordinates::Coordinates& rhs)
			{ return lhs.lat < rhs.lat || (lhs.lat == rhs.lat && lhs.lng < rhs.lng); });
		coords.erase(std::unique(coords.begin(), coords.end()), coords.end());
		std::sort(unique_stops.begin(), unique_stops.end(), [](const Stop* lhs, const Stop* rhs) { return lhs->GetName() < rhs->GetName(); });
		unique_stops.erase(std::unique(unique_stops.begin(), unique_stops.end()), unique_stops.end());

		const SphereProjector projection{ coords.begin(), coords.end(), width, height, padding };

		svg::Document canvas;

		// Routes themselves
		for (Route* route : routes)
		{
			if (route->GetStopsCount() == 0)
				continue;

			AddRouteToCanvas(canvas, route, projection);
		}
		current_pallete_index = 0; // reset incex, so each route could get it again exactly the same

		// Routes names
		for (Route* route : routes)
		{
			if (route->GetStopsCount() == 0)
				continue;

			AddRouteNameToCanvas(canvas, route, projection);
		}

		//Stops themselves
		for (Stop* stop : unique_stops)
		{
			AddStopToCanvas(canvas, projection(stop->GetCoordinates()));
		}

		//Stops names
		for (Stop* stop : unique_stops)
		{
			AddStopNameToCanvas(canvas, stop, projection);
		}

		return canvas;
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

	rend::MapRenderer MapRenderer::Serialize()
	{
		rend::MapRenderer map_renderer;

		map_renderer.set_width(width);
		map_renderer.set_height(height);
		map_renderer.set_padding(padding);
		map_renderer.set_line_width(line_width);
		map_renderer.set_stop_radius(stop_radius);
		map_renderer.set_stop_label_font_size(stop_label_font_size);
		map_renderer.set_bus_label_font_size(bus_label_font_size);
		map_renderer.set_underlayer_width(underlayer_width);

		for(const svg::Color& color : color_pallete)
		{
			(*map_renderer.mutable_pallete()).Add(std::move(SVG_2_Rend(color)));
		}
		
		*map_renderer.mutable_stop_label_offset() = std::move(SVG_2_Rend(stop_label_offset));
		*map_renderer.mutable_bus_label_offset() = std::move(SVG_2_Rend(bus_label_offset));
		*map_renderer.mutable_underlayer_color() = std::move(SVG_2_Rend(underlayer_color));

		return map_renderer;
	}
	void MapRenderer::Deserialize(rend::MapRenderer map_renderer)
	{
		width = map_renderer.width();
		height = map_renderer.height();
		padding = map_renderer.padding();
		line_width = map_renderer.line_width();
		stop_radius = map_renderer.stop_radius();
		stop_label_font_size = map_renderer.stop_label_font_size();
		bus_label_font_size = map_renderer.bus_label_font_size();
		underlayer_width = map_renderer.underlayer_width();

		color_pallete.reserve(map_renderer.pallete_size());
		for(rend::Color c : map_renderer.pallete())
		{
			color_pallete.push_back(std::move(Rend_2_SVG(c)));
		}

		stop_label_offset = std::move(Rend_2_SVG(map_renderer.stop_label_offset()));
		bus_label_offset = std::move(Rend_2_SVG(map_renderer.bus_label_offset()));
		underlayer_color = std::move(Rend_2_SVG(map_renderer.underlayer_color()));
        current_pallete_index = 0;
	}

	void MapRenderer::AddRouteToCanvas(svg::Document& canvas, Route* route, const SphereProjector& projection)
	{
		svg::Polyline line;
		const std::vector<Stop*>& stops = route->GetFullRoute();
		for (auto it = stops.begin(); it != stops.end(); it++)
		{
			line.AddPoint(projection((*it)->GetCoordinates()));
		}

		svg::Color color = GetColorFromPallete();
		line.SetStrokeColor(color);
		line.SetFillColor("none");
		line.SetStrokeWidth(line_width);
		line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		canvas.Add(std::move(line));
	}
	void MapRenderer::AddRouteNameToCanvas(svg::Document& canvas, Route* route, const SphereProjector& projection)
	{
		const std::vector<Stop*>& stops = route->GetStops();
		svg::Color color = GetColorFromPallete();

		svg::Text underlayer;
		underlayer.SetData(route->GetName());
		underlayer.SetPosition(projection((*stops.begin())->GetCoordinates()));
		underlayer.SetOffset(bus_label_offset);
		underlayer.SetFontSize(bus_label_font_size);
		underlayer.SetFontFamily("Verdana");
		underlayer.SetFontWeight("bold");
		underlayer.SetFillColor(underlayer_color);
		underlayer.SetStrokeColor(underlayer_color);
		underlayer.SetStrokeWidth(underlayer_width);
		underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		svg::Text route_name_begin;
		route_name_begin.SetData(route->GetName());
		route_name_begin.SetPosition(projection((*stops.begin())->GetCoordinates()));
		route_name_begin.SetOffset(bus_label_offset);
		route_name_begin.SetFontSize(bus_label_font_size);
		route_name_begin.SetFontFamily("Verdana");
		route_name_begin.SetFontWeight("bold");
		route_name_begin.SetFillColor(color);

		canvas.Add(std::move(underlayer));
		canvas.Add(std::move(route_name_begin));

		if (route->GetRouteType() == Route::Linear && *stops.begin() != *stops.rbegin())
		{
			svg::Text underlayer;
			underlayer.SetData(route->GetName());
			underlayer.SetPosition(projection((*stops.rbegin())->GetCoordinates()));
			underlayer.SetOffset(bus_label_offset);
			underlayer.SetFontSize(bus_label_font_size);
			underlayer.SetFontFamily("Verdana");
			underlayer.SetFontWeight("bold");
			underlayer.SetFillColor(underlayer_color);
			underlayer.SetStrokeColor(underlayer_color);
			underlayer.SetStrokeWidth(underlayer_width);
			underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			svg::Text route_name_end;
			route_name_end.SetData(route->GetName());
			route_name_end.SetPosition(projection((*stops.rbegin())->GetCoordinates()));
			route_name_end.SetOffset(bus_label_offset);
			route_name_end.SetFontSize(bus_label_font_size);
			route_name_end.SetFontFamily("Verdana");
			route_name_end.SetFontWeight("bold");
			route_name_end.SetFillColor(color);

			canvas.Add(std::move(underlayer));
			canvas.Add(std::move(route_name_end));
		}
	}
	void MapRenderer::AddStopToCanvas(svg::Document& canvas, const svg::Point& coord)
	{
		svg::Circle stop;
		stop.SetCenter(coord);
		stop.SetRadius(stop_radius);
		stop.SetFillColor("white");

		canvas.Add(stop);
	}
	void MapRenderer::AddStopNameToCanvas(svg::Document& canvas, Stop* stop, const SphereProjector& projection)
	{
		svg::Text underlayer;
		underlayer.SetData(stop->GetName());
		underlayer.SetPosition(projection(stop->GetCoordinates()));
		underlayer.SetOffset(stop_label_offset);
		underlayer.SetFontSize(stop_label_font_size);
		underlayer.SetFontFamily("Verdana");
		underlayer.SetFillColor(underlayer_color);
		underlayer.SetStrokeColor(underlayer_color);
		underlayer.SetStrokeWidth(underlayer_width);
		underlayer.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		underlayer.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

		svg::Text stop_name;
		stop_name.SetData(stop->GetName());
		stop_name.SetPosition(projection(stop->GetCoordinates()));
		stop_name.SetOffset(stop_label_offset);
		stop_name.SetFontSize(stop_label_font_size);
		stop_name.SetFontFamily("Verdana");
		stop_name.SetFillColor("black");

		canvas.Add(std::move(underlayer));
		canvas.Add(std::move(stop_name));
	}

	svg::Color MapRenderer::GetColorFromPallete()
	{
		int prev_index = current_pallete_index;
		current_pallete_index = (current_pallete_index + 1) % color_pallete.size();
		return color_pallete.at(prev_index);
	}
}