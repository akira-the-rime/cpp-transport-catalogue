#include <cstddef>
#include <iomanip>
#include <sstream>
#include <utility>

#include "svg.h"

namespace svg {
    using namespace std::literals;

// ---------------------------------------- Realization -------------------------------
//                                                                                    +
//                                                                                    + --------------------
// ------------------------------------------------------------------------------------ Auxiliary entities +

    namespace detail {
        std::ostream& Printer::operator()([[maybe_unused]] const std::monostate& none) {
            os << "none"s;
            return os;
        }

        std::ostream& Printer::operator()(const std::string& color) {
            os << color;
            return os;
        }
        
        std::ostream& Printer::operator()(const Rgb& rgb) {
            os << "rgb("s << static_cast<uint16_t>(rgb.red) 
                << ","s << static_cast<uint16_t>(rgb.green) 
                << ","s << static_cast<uint16_t>(rgb.blue) << ")"s;

            return os;
        }

        std::ostream& Printer::operator()(const Rgba& rgba) {
            os << "rgba("s << static_cast<uint16_t>(rgba.red) 
                << ","s << static_cast<uint16_t>(rgba.green)
                << ","s << static_cast<uint16_t>(rgba.blue)
                << ","s << rgba.opacity << ")"s;

            return os;
        }
    }

    std::ostream& operator<<(std::ostream& os, const Color& color) {
        return std::visit(detail::Printer{ os }, color);
    }

// 
// 
//                                                                                    + --------------
// ------------------------------------------------------------------------------------ Enum classes +
    std::ostream& operator<<(std::ostream& os, const StrokeLineCap& line_cap) {
        switch (line_cap) {
        case StrokeLineCap::BUTT:
            os << "butt"s;
            break;
        case StrokeLineCap::ROUND:
            os << "round"s;
            break;
        case StrokeLineCap::SQUARE:
            os << "square"s;
            break;
        }

        return os;
    }

    std::ostream& operator<<(std::ostream& os, const StrokeLineJoin& line_join) {
        switch (line_join) {
        case StrokeLineJoin::ARCS:
            os << "arcs"s;
            break;
        case StrokeLineJoin::BEVEL:
            os << "bevel"s;
            break;
        case StrokeLineJoin::MITER:
            os << "miter"s;
            break;
        case StrokeLineJoin::MITER_CLIP:
            os << "miter-clip"s;
            break;
        case StrokeLineJoin::ROUND:
            os << "round"s;
            break;
        }

        return os;
    }

//
// 
//                                                                                    + --------
// ------------------------------------------------------------------------------------ Render +
    RenderContext::RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext::RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext RenderContext::Indented() const {
        return { out, indent_step, indent + indent_step };
    }

    void RenderContext::RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
                
//
//
//                                                                                    + -----------------------
// ------------------------------------------------------------------------------------ Abstract class Object +
    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();
        RenderObject(context);

        context.out << std::endl;
    }

//
// 
//                                                                                    + --------
// ------------------------------------------------------------------------------------ Circle +
    Circle& Circle::SetCenter(Point center)  noexcept {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)  noexcept {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

//
// 
//                                                                                    + ----------
// ------------------------------------------------------------------------------------ Polyline +
    Polyline& Polyline::AddPoint(Point point) noexcept {
        std::stringstream to_parse;

        to_parse << std::setprecision(6) << point.x << " "s << point.y;
        std::string first;
        std::string second;
        to_parse >> first >> second;
        points_ = points_ + first + ","s + second + " "s;

        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;

        std::stringstream to_write(points_);
        std::string written;
        bool is_first = true;
        while (to_write >> written) {
            is_first ? out << written : out << " "sv << written;
            is_first = false;
        }

        out << "\" "sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

//
// 
//                                                                                    + ------
// ------------------------------------------------------------------------------------ Text +
    Text& Text::SetPosition(Point pos)  noexcept {
        position_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset) noexcept {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size) noexcept {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family) {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight) {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data) {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;

        out << "<text "sv; 
        RenderAttrs(out);

        out << "x=\""sv << position_.x << "\" y=\""sv << position_.y
            << "\" dx=\""sv << offset_.x << "\" dy=\"" << offset_.y
            << "\" font-size=\""sv << font_size_ << "\""sv;
        
        if (font_family_.size()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }

        if (font_weight_.size()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }
        out << ">"sv;

        for (char to_check : data_) {
            switch (to_check) {
            case '\"':
                out << "&quot;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            default:
                out.put(to_check);
            }
        }
        
        out << "</text>"sv;
    }

//
// 
//                                                                                    + ----------
// ------------------------------------------------------------------------------------ Document +
    void Document::AddPtr(std::unique_ptr<Object>&& obj) {
        svgs_.push_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
        for (const auto& object : svgs_) {
            object->Render({ out, 2, 2 });
        }
        out << "</svg>"sv;
    }
}