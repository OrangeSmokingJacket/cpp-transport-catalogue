#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <variant>

namespace svg
{
    struct Rgb
    {
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b)
        {
            red = r;
            green = g;
            blue = b;
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };
    struct Rgba
    {
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b, double alpha)
        {
            red = r;
            green = g;
            blue = b;
            opacity = alpha;
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };
    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    inline const Color NoneColor{ "none" };
    struct ColorPrinter
    {
        std::ostream& out;
        void operator()(std::monostate) const;
        void operator()(std::string color) const;
        void operator()(svg::Rgb color) const;
        void operator()(svg::Rgba color) const;
    };
    std::ostream& operator<<(std::ostream& out, const Color& c);

    enum class StrokeLineCap { BUTT, ROUND, SQUARE };
    std::ostream& operator<<(std::ostream& out, const StrokeLineCap& c);
    enum class StrokeLineJoin { ARCS, BEVEL, MITER, MITER_CLIP, ROUND };
    std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& j);

    struct Point
    {
        Point() = default;
        Point(double x, double y) : x(x), y(y) {}

        double x = 0;
        double y = 0;
    };
    struct RenderContext
    {
        RenderContext(std::ostream& out) : out(out) {}
        RenderContext(std::ostream& out, int indent_step, int indent = 0) : out(out), indent_step(indent_step), indent(indent) {}

        RenderContext Indented() const
        {
            return { out, indent_step, indent + indent_step };
        }
        void RenderIndent() const
        {
            for (int i = 0; i < indent; i++)
            {
                out.put(' ');
            }
        }

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    template <typename Owner>
    class PathProps
    {
    public:
        Owner& SetFillColor(Color color)
        {
            fill_color = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color)
        {
            stroke_color = std::move(color);
            return AsOwner();
        }
        Owner& SetStrokeWidth(double width)
        {
            stroke_width = std::move(width);
            return AsOwner();
        }
        Owner& SetStrokeLineCap(StrokeLineCap cap)
        {
            line_cap = std::move(cap);
            return AsOwner();
        }
        Owner& SetStrokeLineJoin(StrokeLineJoin join)
        {
            line_join = std::move(join);
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(std::ostream& out) const
        {
            using namespace std::literals;

            if (fill_color)
                out << " fill=\""sv << *fill_color << "\""sv;
            if (stroke_color)
                out << " stroke=\""sv << *stroke_color << "\""sv;
            if (stroke_width)
                out << " stroke-width=\""sv << *stroke_width << "\""sv;
            if (line_cap)
                out << " stroke-linecap=\""sv << *line_cap << "\""sv;
            if (line_join)
                out << " stroke-linejoin=\""sv << *line_join << "\""sv;
        }

    private:
        Owner& AsOwner()
        {
            return static_cast<Owner&>(*this);
        }

        std::optional<Color> fill_color;
        std::optional<Color> stroke_color;
        std::optional<double> stroke_width;
        std::optional<StrokeLineCap> line_cap;
        std::optional<StrokeLineJoin> line_join;
    };
    class Object
    {
    public:
        void Render(const RenderContext& context) const;

        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };
    class ObjectContainer
    {
    public:
        template <typename Obj>
        void Add(Obj obj)
        {
            AddPtr(std::make_unique<Obj>(std::move(obj)));
        }
        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    private:
        std::vector<std::unique_ptr<Object>> objects;
    };
    class Drawable
    {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

    class Circle : public Object, public PathProps<Circle>
    {
    public:
        Circle& SetCenter(Point c);
        Circle& SetRadius(double r);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center;
        double radius = 1.0;
    };
    class Polyline : public Object, public PathProps<Polyline>
    {
    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> points;
    };
    class Text : public Object, public PathProps<Text>
    {
    public:
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(uint32_t size);
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);

    private:
        std::string ConvertData(const std::string& data);
        void RenderObject(const RenderContext& context) const override;

        Point pos;
        Point offset;
        uint32_t size = 1;
        std::optional<std::string> font_family;
        std::optional<std::string> font_weight;
        std::string data;
    };

    class Document : public ObjectContainer
    {
    public:
        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> objects_;
    };

}  // namespace svg