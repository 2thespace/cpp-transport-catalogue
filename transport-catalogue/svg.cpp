#include "svg.h"

namespace svg {

    using namespace std::literals;
    std::ostream& operator<<(std::ostream& out, StrokeLineCap line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, StrokeLineJoin line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        }
        return out;
    }
    std::ostream& operator<<(std::ostream& out, const Color& color) {
        std::visit(ColorPrinter{ out }, color);
        return out;
    }

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << std::endl;
    }

    // ---------- Circle ------------------
    svg::Point Circle::GetCenter(void) {
        return this->center_;
    }

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "  <circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }
    // ---------- Ellipse ------------------
    Ellipse& Ellipse::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Ellipse& Ellipse::SetRadius(double radius_x, double radius_y)
    {
        radius_x_ = radius_x;
        radius_y_ = radius_y;
        return *this;
    }

    void Ellipse::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<ellipse cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "rx=\""sv << radius_x_ << "\" "sv;
        out << "ry=\""sv << radius_y_ << "\" "sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }
    Polyline& Polyline::AddPoint(Point point)
    {
        points_.push_back(point);
        return *this;
    }
    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "  <polyline points=\""sv;
        bool is_first = true;
        for (auto& point : points_)
        {
            if (!is_first) {
                out << " "sv;
            }
            else {
                is_first = false;
            }
            out << point.x << ","sv << point.y;
        }

        out << "\""sv;
        RenderAttrs(context.out);
        out << "/>"sv;
    }

    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& Text::SetPosition(Point pos)
    {
        point_pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset)
    {
        point_offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size)
    {
        font_size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(std::string data)
    {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        
        out << "  <text"sv;
        RenderAttrs(context.out);
        out << " x=\"" << point_pos_.x << "\" y=\""sv << point_pos_.y << "\" "sv;
        out << "dx=\""sv << point_offset_.x << "\" dy=\""sv << point_offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if (!font_family_.empty()) out << " font-family=\""sv << font_family_ << "\""sv;
        if (!font_weight_.empty()) out << " font-weight=\""sv << font_weight_ << "\""sv;
        
        out << ">"sv << data_ << "</text>"sv;
    }


    // Добавляет в svg-документ объект-наследник svg::Object
    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        objects_.emplace_back(std::move(obj));
    }

    // Выводит в ostream svg-представление документа
    void Document::Render(std::ostream& out) const
    {
        RenderContext context(out);
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& obj : objects_) {
            obj->Render(context);
        }
        out << "</svg>"sv;
    }


}  // namespace svg