#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <list>
#include <vector>
#include <optional>
#include <variant>

namespace svg 
{

struct Rgb
{
    Rgb() = default;
    Rgb(uint8_t red, uint8_t green, uint8_t blue): red(red), green(green), blue(blue) {}
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct Rgba
{
    Rgba() = default;
    Rgba(uint8_t red, uint8_t green, uint8_t blue, double opacity): red(red), green(green), blue(blue), opacity(opacity) {}
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

// using Color = std::string;
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{};

struct PrintColor
{ // вывод содержимого variant
    PrintColor(std::ostream& os) : os(os) {}
    std::ostream& operator()(std::monostate) const;
    std::ostream& operator()(const std::string& color) const;
    std::ostream& operator()(const Rgb& color) const;
    std::ostream& operator()(const Rgba& color) const;

    std::ostream& os;
};

std::ostream& operator<<(std::ostream& os, const Color& color);

/*
Задает тип формы конца строки
https://www.w3.org/TR/SVG/painting.html#StrokeLinecapProperty
*/
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

std::ostream& operator<<(std::ostream& os, const StrokeLineCap cap);

/*
Задает  тип формы соединения линий
https://www.w3.org/TR/SVG/painting.html#StrokeLinejoinProperty
*/
enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin line_join); 

struct Point 
{ // структура описывающая точку на кординатной плоскости, 0 0 - левый верхний угол, но увеличение по ости Y это отступ вниз
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;

    bool operator==(const Point other) const;
    bool operator!=(const Point other) const;
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext 
{
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object 
{
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    
    virtual void RenderObject(const RenderContext& context) const = 0;
};

template <typename Owner>
class PathProps 
{ // интерфейс задания свойств форматирования, таких как цвет и форма обводки
public:

    PathProps() = default;
    Owner& SetFillColor(Color color) 
    {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) 
    {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width)
    {
        stroke_width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap)
    {
        stroke_line_cap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join)
    {
        stroke_line_join_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;



    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (stroke_width_) 
        {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (stroke_line_cap_)
        {
            out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
        }
        if (stroke_line_join_)
        {
            out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        // static_cast безопасно преобразует *this к Owner&,
        // если класс Owner — наследник PathProps
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> 
{
public:
    Circle();

    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline: public Object, public PathProps<Polyline> 
{
public:
    Polyline() = default;
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

    /*
     * Прочие методы и данные, необходимые для реализации элемента <polyline>
     */
private:
    
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text: public Object, public PathProps<Text>
{
public:
    Text() = default;
    // Задаёт координаты опорной точки (атрибуты x и y)
    Text& SetPosition(Point pos);

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& SetOffset(Point offset);

    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);

    // Задаёт название шрифта (атрибут font-family)
    Text& SetFontFamily(std::string font_family);

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(std::string font_weight);

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(std::string data);

    const Point& GetPosition() const;

private:

    void RenderObject(const RenderContext& context) const override;

    Point position_{0.0, 0.0};
    Point offset_{0.0, 0.0};
    uint32_t font_size_ = 1;
    std::string font_family_{};
    std::string font_weight_{};
    std::string data_{};
};

class ObjectContainer
{ // интерфейс для унифицированного добавления 
private:
public:
    virtual void AddPtr(std::unique_ptr<Object>&& object) = 0;

    template <typename T>
    void Add(T object) 
    {
        AddPtr(std::make_unique<T>(object));
    }

    virtual ~ObjectContainer() = default;
}; // ObjectContaibetr

class Drawable 
{ // интерфейс для унификации метода рисования объектов
private:
public:
    virtual void Draw(svg::ObjectContainer& contaiber) const = 0;
    virtual ~Drawable() = default;
}; // Drawable

class Document: public ObjectContainer 
{ 
public:

    Document() = default;

    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

    // Прочие методы и данные, необходимые для реализации класса Document

private:

    std::list<std::unique_ptr<Object>> objects_;
};

}  // namespace svg