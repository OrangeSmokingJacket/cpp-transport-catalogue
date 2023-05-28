#include "svg.h"

namespace svg
{
    using namespace std::literals;

    // ---------- ColorPrinter ------------

    void ColorPrinter::operator()(std::monostate) const
    {
        out << "none";
    }
    void ColorPrinter::operator()(std::string color) const
    {
        out << color;
    }
    void ColorPrinter::operator()(svg::Rgb color) const
    {
        out << "rgb(" << std::to_string(color.red) << "," << std::to_string(color.green) << "," << std::to_string(color.blue) << ")";
    }
    void ColorPrinter::operator()(svg::Rgba color) const
    {
        out << "rgba(" << std::to_string(color.red) << "," << std::to_string(color.green) << "," << std::to_string(color.blue) << "," << color.opacity << ")";
    }

    std::ostream& operator<<(std::ostream& out, const Color& c)
    {
        std::visit(ColorPrinter{ out }, c);
        return out;
    }
    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& c)
    {
        switch (c)
        {
        case svg::StrokeLineCap::BUTT:
            out << "butt";
            break;
        case svg::StrokeLineCap::ROUND:
            out << "round";
            break;
        case svg::StrokeLineCap::SQUARE:
            out << "square";
            break;
        }
        return out;
    }
    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& j)
    {
        switch (j)
        {
        case svg::StrokeLineJoin::ARCS:
            out << "arcs";
            break;
        case svg::StrokeLineJoin::BEVEL:
            out << "bevel";
            break;
        case svg::StrokeLineJoin::MITER:
            out << "miter";
            break;
        case svg::StrokeLineJoin::MITER_CLIP:
            out << "miter-clip";
            break;
        case svg::StrokeLineJoin::ROUND:
            out << "round";
            break;
        }
        return out;
    }

    void Object::Render(const RenderContext& context) const
    {
        context.RenderIndent();

        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point c)
    {
        center = std::move(c);
        return *this;
    }
    Circle& Circle::SetRadius(double r)
    {
        radius = std::move(r);
        return *this;
    }
    void Circle::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<circle cx=\""sv << center.x << "\" cy=\""sv << center.y << "\" "sv;
        out << "r=\""sv << radius << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ----------------

    Polyline& Polyline::AddPoint(Point point)
    {
        points.push_back(std::move(point));
        return *this;
    }
    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < points.size(); i++)
        {
            if (i != 0)
                out << " ";

            out << points[i].x << ","sv << points[i].y;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text --------------------

    Text& Text::SetPosition(Point pos)
    {
        (*this).pos = std::move(pos);
        return *this;
    }
    Text& Text::SetOffset(Point offset)
    {
        (*this).offset = std::move(offset);
        return *this;
    }
    Text& Text::SetFontSize(uint32_t size)
    {
        (*this).size = std::move(size);
        return *this;
    }
    Text& Text::SetFontFamily(std::string font_family)
    {
        (*this).font_family = std::move(font_family);
        return *this;
    }
    Text& Text::SetFontWeight(std::string font_weight)
    {
        (*this).font_weight = std::move(font_weight);
        return *this;
    }
    Text& Text::SetData(std::string data)
    {
        (*this).data = std::move(ConvertData(std::move(data)));
        return *this;
    }

    std::string Text::ConvertData(const std::string& data)
    {
        std::string result = std::move(data);
        size_t pos = result.find('&');
        while (pos != std::string::npos)
        {
            result.replace(pos, 1, "&amp;");
            pos = result.find('&', pos + 1);
        }
        pos = result.find('<');
        while (pos != std::string::npos)
        {
            result.replace(pos, 1, "&lt;");
            pos = result.find('<', pos + 1);
        }
        pos = result.find('>');
        while (pos != std::string::npos)
        {
            result.replace(pos, 1, "&gt;");
            pos = result.find('>', pos + 1);
        }
        pos = result.find("\"");
        while (pos != std::string::npos)
        {
            result.replace(pos, 1, "&quot;");
            pos = result.find("\"", pos + 1);
        }
        pos = result.find("\'");
        while (pos != std::string::npos)
        {
            result.replace(pos, 1, "&apos;");
            pos = result.find("\'", pos + 1);
        }

        return result;
    }
    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;

        out << "<text";
        RenderAttrs(out);

        out << " x=\"" << pos.x << "\" y=\"" << pos.y;
        out << "\" dx=\"" << offset.x << "\" dy=\"" << offset.y;
        out << "\" font-size=\"" << size << "\"";

        if (font_family)
            out << " font-family=\"" << *font_family << "\"";
        if (font_weight)
            out << " font-weight=\"" << *font_weight << "\"";

        out << ">" << data << "</text>";
    }

    // ---------- Document ----------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        objects_.emplace_back(std::move(obj));
    }
    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (size_t i = 0; i < objects_.size(); i++)
        {
            //out << "  ";
            (*objects_[i]).Render(out);
        }
        out << "</svg>"sv;
    }
}