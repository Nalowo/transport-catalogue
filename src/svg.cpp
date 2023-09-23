#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& os, const Color& color)
{
    PrintColor out{os};
    return std::visit(out, color);
}

std::ostream& operator<<(std::ostream& os, const StrokeLineCap cap)
{
    switch (cap) {
    case StrokeLineCap::BUTT:
        return os << "butt";
    case StrokeLineCap::ROUND:
        return os << "round";
    case StrokeLineCap::SQUARE:
        return os << "square";

    }
    return os;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin line_join) 
    { 
    using namespace std::literals; 
    switch(line_join) 
    { 
        case StrokeLineJoin::ARCS : 
            return out << "arcs"s;
        case StrokeLineJoin::BEVEL : 
            return out << "bevel"s; 
        case StrokeLineJoin::MITER : 
            return out << "miter"s; 
        case StrokeLineJoin::MITER_CLIP : 
            return out << "miter-clip"s; 
        case StrokeLineJoin::ROUND : 
            return out << "round"s; 
    }
    return out; 
} 

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// --------------Point----------------
bool Point::operator==(const Point other) const
{
    return this->x == other.x && this->y == other.y;
}

bool Point::operator!=(const Point other) const
{
    return !(*this == other);
}


// ----------PrintColor----------------

std::ostream& PrintColor::operator()(std::monostate) const 
{
    return os << "none";
}
std::ostream& PrintColor::operator()(const std::string& color) const
{
    return os << color;
}
std::ostream& PrintColor::operator()(const Rgb& color) const 
{
    return os << "rgb(" << (unsigned) color.red << "," << (unsigned) color.green << "," << (unsigned) color.blue << ")";
}
std::ostream& PrintColor::operator()(const Rgba& color) const 
{
    return os << "rgba(" << (unsigned) color.red << "," << (unsigned) color.green << "," << (unsigned) color.blue << "," << color.opacity << ")";
}

// ---------- Circle ------------------

Circle::Circle(): center_({0.0, 0.0}) {}

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const 
{
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

// -----------Document-----------------

void Document::AddPtr(std::unique_ptr<Object>&& object) {
    objects_.push_back(std::move(object));
    // objects_.push_back(std::move(*object.get()));
}

void Document::Render(std::ostream& out) const 
{
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << '\n';
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << '\n';
    
    for (const auto& object : objects_) 
    {
        object->Render(RenderContext{out, 1, 2});
    }
    
    out << "</svg>"sv;
}

// -----------Polyline-----------------

Polyline& Polyline::AddPoint(Point point)
{
    points_.push_back(std::move(point));

    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const 
{
    auto& out = context.out;
    out << "<polyline points=\""sv;
    if (!points_.empty())
    {
        bool spece_begin_flag = false;
        for (auto& point : points_) 
        {
            if (spece_begin_flag)
            {
                out << " "sv;
            }
            out << point.x << ","sv << point.y;
            spece_begin_flag = true;
        }
    }
    else
    {
        out << ""sv;
    }
    out << "\""sv;
    this->RenderAttrs(context.out);
    out << "/>"sv;
}

// --------------Text-------------------

const Point& Text::GetPosition() const
{
    return position_;
}

Text& Text::SetPosition(Point position)
{
    position_ = position;

    return *this;
}

Text& Text::SetOffset(Point offset)
{
    offset_ = offset;

    return *this;
}

Text& Text::SetFontSize(uint32_t font_size)
{
    font_size_ = font_size;

    return *this;
}

Text& Text::SetFontFamily(std::string font_family)
{
    font_family_ = std::move(font_family);

    return *this;
}

Text& Text::SetFontWeight(std::string font_weight)
{
    font_weight_ = std::move(font_weight);

    return *this;
}

Text& Text::SetData(std::string text)
{
    data_ = std::move(text);

    return *this;
}

void Text::RenderObject(const RenderContext& context) const 
{
    auto& out = context.out;
    out << "<text";
    this->RenderAttrs(context.out);
    out << " x=\""sv << position_.x << "\" y=\""sv << position_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << font_size_ << "\" "sv;
    if (!font_family_.empty())
    {
        out << "font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty())
    {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv;
    for (char sign : data_) 
    {
        switch (sign)
        {
        case '\"':
            out << "&quot;"sv;
            break;
        case '&':
            out << "&amp;"sv;
            break;
        case '<':
            out << "&lt;"sv;
            break;
        case '>':
            out << "&gt;"sv;
            break;
        case '\'':
            out << "&apos;"sv;
            break;
        default:
            out << sign;
            break;
        }
        
    }
    out << "</text>"sv;
}

}  // namespace svg