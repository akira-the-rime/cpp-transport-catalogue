#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {
    using namespace std::literals;

// ------------ Interfaces ------------
//                                    +
//                                    + ---------------------------
// ------------------------------------ ObjectContainer Interface +

    class Object;

    class ObjectContainer { 
    public:
        template<class Type>
        void Add(const Type& svg);

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
        virtual ~ObjectContainer() = default;
    };

    template<class Type>
    void ObjectContainer::Add(const Type& svg) {
        AddPtr(std::make_unique<Type>(svg));
    }

// 
// 
//                                    + --------------------
// ------------------------------------ Drawable Interface +

    class Drawable {
    public:
        virtual void Draw(ObjectContainer& container) const = 0;
        virtual ~Drawable() = default;
    };

// ------------ Realization ------------
//                                     +
//                                     + --------------------
// ------------------------------------- Auxiliary entities +

    struct Rgb final {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    struct Rgba final {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = std::variant<std::monostate, std::string, svg::Rgb, svg::Rgba>;
    inline const Color NoneColor{ "none"s };

    namespace detail {
        struct Printer final {
            std::ostream& os;

            std::ostream& operator()([[maybe_unused]] const std::monostate& none);
            std::ostream& operator()(const std::string& color);
            std::ostream& operator()(const Rgb& rgb);
            std::ostream& operator()(const Rgba& rgba);
        };
    }

    std::ostream& operator<<(std::ostream& os, const Color& color);

//         
// 
//                                     + -------------------
// ------------------------------------- Point coordinates +

    struct Point final { 
        Point() = default;

        Point(double x, double y) noexcept
            : x(x)
            , y(y) {
        }

        double x = 0.0;
        double y = 0.0;
    };

//
//                                                                                    
//                                     + --------------
// ------------------------------------- Enum classes +

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap);
    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join);

//
//
//                                     + --------
// ------------------------------------- Render +

    struct RenderContext final {
        RenderContext(std::ostream& out);
        RenderContext(std::ostream& out, int indent_step, int indent);
        RenderContext Indented() const;
        void RenderIndent() const;

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

//
// 
//                                     + ----------------
// ------------------------------------- PathProperties +

    template <class Owner>
    class PathProps {
    public:
        Owner& SetFillColor(Color fill_color) {
            fill_color_ = fill_color;
            return CheckTypeValidity();
        }

        Owner& SetStrokeColor(Color stroke_color) {
            stroke_color_ = stroke_color;
            return CheckTypeValidity();
        }

        Owner& SetStrokeWidth(double stroke_width) noexcept {
            stroke_width_ = stroke_width;
            return CheckTypeValidity();
        }

        Owner& SetStrokeLineCap(StrokeLineCap stroke_linecap) noexcept {
            stroke_linecap_ = stroke_linecap;
            return CheckTypeValidity();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin stroke_linejoin) noexcept {
            stroke_linejoin_ = stroke_linejoin;
            return CheckTypeValidity();
        }

    protected:
        Owner& CheckTypeValidity() {
            return static_cast<Owner&>(*this);
        }

        void RenderAttrs(std::ostream& out) const noexcept {

            if (fill_color_) {
                out << "fill=\""s << fill_color_.value() << "\" "s;
            }

            if (stroke_color_) {
                out << "stroke=\""s << stroke_color_.value() << "\" "s;
            }

            if (stroke_width_) {
                out << "stroke-width=\""s << stroke_width_.value() << "\" "s;
            }

            if (stroke_linecap_) {
                out << "stroke-linecap=\""s << stroke_linecap_.value() << "\" "s;
            }

            if (stroke_linejoin_) {
                out << "stroke-linejoin=\""s << stroke_linejoin_.value() << "\" "s;
            }
        }

    private:
        std::optional<Color> fill_color_;
        std::optional<Color> stroke_color_;
        std::optional<double> stroke_width_;
        std::optional<StrokeLineCap> stroke_linecap_;
        std::optional<StrokeLineJoin> stroke_linejoin_;
    };

//
// 
//                                     + -----------------------
// ------------------------------------- Abstract class Object +

    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext&) const = 0;
    };

//
// 
//                                     + --------
// ------------------------------------- Circle +

    class Circle final : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center) noexcept;
        Circle& SetRadius(double radius) noexcept;

    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;     
        double radius_ = 1.0;
    };

//
// 
//                                     + ----------
// ------------------------------------- Polyline +

    class Polyline final : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point) noexcept;

    private:
        void RenderObject(const RenderContext& context) const override;
        
        std::string points_ = ""s;
    };

//
// 
//                                     + ------
// ------------------------------------- Text +

    class Text final : public Object, public PathProps<Text> {
    public:
        Text& SetPosition(Point pos) noexcept;
        Text& SetOffset(Point offset) noexcept;
        Text& SetFontSize(uint32_t size) noexcept;
        Text& SetFontFamily(std::string font_family);
        Text& SetFontWeight(std::string font_weight);
        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;

        Point position_;
        Point offset_;
        uint32_t font_size_ = 1;
        std::string font_family_ = ""s;
        std::string font_weight_ = ""s;
        std::string data_ = ""s;
    };

//
// 
//                                     + ----------
// ------------------------------------- Document +

    class Document final : public ObjectContainer {
    public:
        Document() = default;

        void AddPtr(std::unique_ptr<Object>&& obj) override;
        void Render(std::ostream& out) const;

    private:
        std::vector<std::unique_ptr<Object>> svgs_;
    };
} // svg