#pragma once

#include "svg.h"
#include "domain.h"
#include "map_renderer.pb.h"

#include <fstream>

namespace renderer
{
	class SphereProjector;
	
	class MapRenderer
	{
	public:
		void SetWidth(double w);
		void SetHeight(double h);
		void SetPadding(double p);
		void SetLineWidth(double lw);
		void SetStopRadius(double r);
		void SetStopLabelFontSize(int size);
		void SetStopLabelOffset(double dx, double dy);
		void SetBusLabelFontSize(int size);
		void SetBusLabelOffset(double dx, double dy);
		void SetUnderlayerColor(svg::Color color);
		void SetUnderlayerWidth(double ulw);
		void AddToColorPallete(svg::Color color);

		svg::Document CreateCanvas(std::vector<Route*> routes);

		rend::MapRenderer Serialize();
		void Deserialize(rend::MapRenderer map_renderer);

	private:
		void AddRouteToCanvas(svg::Document& canvas, Route* route, const SphereProjector& projection);
		void AddRouteNameToCanvas(svg::Document& canvas, Route* route, const SphereProjector& projection);
		void AddStopToCanvas(svg::Document& canvas, const svg::Point& coord);
		void AddStopNameToCanvas(svg::Document& canvas, Stop* stop, const SphereProjector& projection);

		svg::Color GetColorFromPallete();

		double width = 1;
		double height = 1;
		double padding = 0;
		double line_width = 1;
		double stop_radius = 1;
		int stop_label_font_size = 1;
		svg::Point stop_label_offset = {0,0};
		int bus_label_font_size = 1;
		svg::Point bus_label_offset = {0,0};
		svg::Color underlayer_color = "none";
		double underlayer_width = 1;
		std::vector<svg::Color> color_pallete;
		int current_pallete_index = 0;
	};
}