#include "json.h"

using namespace std;

namespace json {
// ------------------------------------ [JSON] Realization ----------------------------
//                                                                                    +
//                                                                                    + -----------------
// ------------------------------------------------------------------------------------ Inner namespace +
    namespace {
        Node LoadNode(istream& input);

    // ----------------------------------- [Loaders] Realization --------------------------
    //                                                                                    +
    //                                                                                    + --------------------
    // ------------------------------------------------------------------------------------ "E"-part Processor +
        Node ProcessE(istream& input, const std::string& to_convert) {
            bool is_positive_mult;
            std::size_t multiplier = 1;

            if (input.peek() == '-') {
                is_positive_mult = false;
                input.get();
            }
            else if (input.peek() == '+') {
                is_positive_mult = true;
                input.get();
            }
            else {
                is_positive_mult = true;
            }

            std::string factor;
            for (char ch = input.peek(); ch != ',' && ch != ' ' && ch != ']' && ch != '}' && ch != EOF; ch = input.peek()) {
                if (!isdigit(ch)) {
                    throw ParsingError("Parsing Error [int, double]. Invalid \"E\"-part.");
                }

                factor += input.get();
            }

            for (int i = 0; i < std::stoi(factor); ++i) {
                multiplier *= 10;
            }

            return is_positive_mult
                ? Node(std::stod(to_convert) * multiplier)
                : Node(std::stod(to_convert) / multiplier);
        }

    // 
    // 
    //                                                                                    + ---------------------
    // ------------------------------------------------------------------------------------ Int & Double Loader +
        Node LoadIntAndDouble(istream& input) {
            bool is_integer = true;
            std::string to_convert;

            if (char ch = input.peek(); ch == '-') {
                to_convert.push_back(ch);
                input.get();
            }

            for (char ch = input.peek(); ch != ',' && ch != ' ' && ch != ']' && ch != '}' && ch != EOF; ch = input.peek()) {
                switch (ch) {
                case '.': {
                    if (is_integer) {
                        to_convert.push_back(input.get());

                        is_integer = false;
                        break;
                    }

                    throw ParsingError("Parsing Error [int, double]. Decimal points' quantity is bigger than one.");
                }
                case 'e':
                    input.get();
                    return ProcessE(input, to_convert);

                case 'E':
                    input.get();
                    return ProcessE(input, to_convert);

                default:
                    if (isdigit(ch)) {
                        to_convert.push_back(input.get());
                        break;
                    }
                    else if (ch == '\n' || ch == '\r' || ch == '\"' || ch == '\t' || ch == '\\') {
                        input.get();
                        break;
                    }

                    throw ParsingError("Parsing Error [int, double]. An unexpected character has been spotted.");
                }
            }

            return is_integer
                ? Node(std::stoi(to_convert))
                : Node(std::stod(to_convert));
        }

    // 
    // 
    //                                                                                    + ----------------
    // ------------------------------------------------------------------------------------ Boolean Loader +
        Node LoadBool(istream& input) {
            std::string to_check;
            const std::size_t size_of_true = 4;

            for (std::size_t i = 0; i < size_of_true; ++i) {
                to_check.push_back(input.get());
            }

            char ch = ' ';
            input >> ch;
            if ((ch == ',' || ch == ']' || ch == '}' || ch == ' ') && to_check == "true"s) {

                if (ch != ' ') {
                    input.putback(ch);
                }
                return Node{ true };
            }

            to_check.push_back(ch);
            ch = ' ';
            input >> ch;
            if (to_check == "false"s && (ch == ',' || ch == ']' || ch == '}' || ch == ' ')) {

                if (ch != ' ') {
                    input.putback(ch);
                }
                return Node{ false };
            }


            throw ParsingError("Parsing Error [boolean].");
        }

    // 
    // 
    //                                                                                    + ---------------
    // ------------------------------------------------------------------------------------ String Loader +
        Node LoadString(istream& input) {
            std::string line;

            for (char ch = input.peek(); !input.eof(); ch = input.peek()) {
                if (ch == '"') {
                    input.get();
                    return Node(std::move(line));
                }
                if (ch == '\\') {
                    ch = input.get();

                    switch (input.peek()) {
                    case 'n':
                        line.push_back('\n');
                        input.get();
                        break;

                    case 'r':
                        line.push_back('\r');
                        input.get();
                        break;

                    case '\"':
                        line.push_back('\"');
                        input.get();
                        break;

                    case 't':
                        line.push_back('\t');
                        input.get();
                        break;

                    case '\\':
                        line.push_back('\\');
                        input.get();
                        break;

                    default:
                        line.push_back(ch);
                    }

                    continue;
                }

                line.push_back(input.get());
            }

            throw ParsingError("Parsing Error [string].");
        }

    // 
    // 
    //                                                                                    + ------------------
    // ------------------------------------------------------------------------------------ Nullptr_t Loader +
        Node LoadNull(istream& input) {
            std::string to_check;
            const std::size_t size_of_null = 4;

            for (std::size_t i = 0; i < size_of_null; ++i) {
                to_check.push_back(input.get());
            }

            char ch = ' ';
            input >> ch;
            if (to_check == "null"s && (ch == ',' || ch == ']' || ch == '}' || ch == ' ')) {

                if (ch != ' ') {
                    input.putback(ch);
                }
                return Node{ nullptr };
            }

            throw ParsingError("Parsing Error [null].");
        }

    // 
    // 
    //                                                                                    + --------------
    // ------------------------------------------------------------------------------------ Array Loader +
        Node LoadArray(istream& input) {
            Array result;

            char ch = '~';
            while (input >> ch && ch != ']') {
                if (ch != ',') {
                    input.putback(ch);
                }

                result.push_back(LoadNode(input));
            }

            if (ch != ']') {
                throw ParsingError("Parsing Error [Array]. \"]\" has not been spotted.");
            }

            return Node(std::move(result));
        }

    // 
    // 
    //                                                                                    + -------------------
    // ------------------------------------------------------------------------------------ Dictionary Loader +
        Node LoadDict(std::istream& input) {
            Dict result;

            for (char ch; input >> ch && ch != '}';) {
                if (ch == '"') {
                    std::string key = LoadString(input).AsString();

                    if (input >> ch && ch == ':') {
                        if (result.find(key) != result.end()) {
                            throw ParsingError("Parsing Error [Dict]. Duplicate key '"s + key + "' has been spotted.");
                        }

                        result.emplace(std::move(key), LoadNode(input));
                    }
                    else {
                        throw ParsingError("Parsing Error [Dict]. Sign \":\" was expected but '"s + ch + "' has been spotted."s);
                    }
                }
                else if (ch != ',') {
                    throw ParsingError("Parsing Error [Dict]. Sign \",\" was expected but '"s + ch + "' has been spotted."s);
                }
            }

            if (!input) {
                throw ParsingError("Parsing Error [Dict]."s);
            }

            return Node(std::move(result));
        }

    // 
    // 
    //                                                                                    + -------------
    // ------------------------------------------------------------------------------------ Node Loader +
        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == '-' || isdigit(c)) {
                input.putback(c);
                return LoadIntAndDouble(input);
            }

            throw ParsingError("Parsing Error [char]. Invalid type has been spotted.");
        }
    } // unnamed namespace

// ----------------------------------- [Node] Realization -----------------------------
//                                                                                    +
//                                                                                    + -------------------
// ------------------------------------------------------------------------------------ Node Constructors +
    Node::Node(int value) noexcept
        : json_lib_(value) {
    }

    Node::Node(double value) noexcept
        : json_lib_(value) {
    }

    Node::Node(bool boolean) noexcept
        : json_lib_(boolean) {
    }

    Node::Node(string str)
        : json_lib_(std::move(str)) {
    }

    Node::Node(std::nullptr_t null) noexcept
        : json_lib_(null) {
    }

    Node::Node(Array array)
        : json_lib_(std::move(array)) {
    }

    Node::Node(Dict map)
        : json_lib_(std::move(map)) {
    }

// 
// 
//                                                                                    + --------------
// ------------------------------------------------------------------------------------ "Is" methods +
    bool Node::IsInt() const noexcept {
        return std::holds_alternative<int>(json_lib_);
    }

    bool Node::IsDouble() const noexcept {
        return std::holds_alternative<int>(json_lib_) || std::holds_alternative<double>(json_lib_);
    }

    bool Node::IsPureDouble() const noexcept {
        return std::holds_alternative<double>(json_lib_);
    }

    bool Node::IsBool() const noexcept {
        return std::holds_alternative<bool>(json_lib_);
    }

    bool Node::IsString() const noexcept {
        return std::holds_alternative<std::string>(json_lib_);
    }

    bool Node::IsNull() const noexcept {
        return std::holds_alternative<std::nullptr_t>(json_lib_);
    }

    bool Node::IsArray() const noexcept {
        return std::holds_alternative<Array>(json_lib_);
    }

    bool Node::IsMap() const noexcept {
        return std::holds_alternative<Dict>(json_lib_);
    }

//
//
//                                                                                    + --------------
// ------------------------------------------------------------------------------------ "As" methods +
    int Node::AsInt() const {
        return *Getter<int>();
    }

    double Node::AsDouble() const {
        try {
            if (std::holds_alternative<int>(json_lib_)) {
                return std::get<int>(json_lib_);
            }

            return std::get<double>(json_lib_);
        }
        catch (const std::bad_variant_access& ex) {
            throw std::logic_error("Wrong type has been requested.");
        }
    }

    bool Node::AsBool() const {
        return *Getter<bool>();
    }

    const string& Node::AsString() const {
        return *Getter<std::string>();
    }

    const Array& Node::AsArray() const {
        return *Getter<Array>();
    }

    const Dict& Node::AsMap() const {
        return *Getter<Dict>();
    }

// 
// 
//                                                                                    + ----------------
// ------------------------------------------------------------------------------------ Node operators +
    bool Node::operator==(const Node& other) const {
        return this->json_lib_ == other.json_lib_;
    }

    bool Node::operator!=(const Node& other) const {
        return this->json_lib_ != other.json_lib_;
    }

// -------------------------- [Storage & Main Loader] Realization ---------------------
//                                                                                    +
//                                                                                    + -----------------------
// ------------------------------------------------------------------------------------ Storage & Main Loader +
    Document::Document(Node root)
        : root_(std::move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

// 
// 
//                                                                                    + --------------------
// ------------------------------------------------------------------------------------ Document operators +
    bool Document::operator==(const Document& other) const {
        return this->root_ == other.root_;
    }

    bool Document::operator!=(const Document& other) const {
        return this->root_ != other.root_;
    }

// --------------------------------- [Printers] Realization ---------------------------
//                                                                                    +
//                                                                                    + ----------------
// ------------------------------------------------------------------------------------ String printer +
    void PrintString(const Document& doc, std::ostream& output) {
        output << "\"";

        for (const auto& ch : doc.GetRoot().AsString()) {
            switch (ch) {
            case '\n':
                output << "\\n"s;
                break;

            case '\r':
                output << "\\r"s;
                break;

            case '\"':
                output << "\\\""s;
                break;

            case '\t':
                output << "\\t"s;
                break;

            case '\\':
                output << "\\\\"s;
                break;

            default:
                output << ch;
            }
        }

        output << "\"";
    }

// 
// 
//                                                                                    + ---------------
// ------------------------------------------------------------------------------------ Array printer +
    void PrintArray(const Document& doc, std::ostream& output) {
        output << "["s;

        bool is_first = true;
        for (const auto& entity : doc.GetRoot().AsArray()) {
            if (is_first) {
                Print(Document{ entity }, output);
                is_first = false;

                continue;
            }

            output << ", "s;
            Print(Document{ entity }, output);
        }

        output << "]"s;
    }

// 
// 
//                                                                                    + --------------------
// ------------------------------------------------------------------------------------ Dictionary printer +
    void PrintMap(const Document& doc, std::ostream& output) {
        output << "{ "s << std::endl;

        bool is_first = true;
        for (const auto& [str, entity] : doc.GetRoot().AsMap()) {
            if (is_first) {
                output << "  "s;
                Print(Document{ str }, output);

                output << " : "s;

                Print(Document{ entity }, output);
                is_first = false;

                continue;
            }
            output << ", "s << std::endl;
            output << "  "s;
            Print(Document{ str }, output);

            output << " : "s;

            Print(Document{ entity }, output);
        }

        output << std::endl << "}"s;
    }

// 
// 
//                                                                                    + -----------------------
// ------------------------------------------------------------------------------------ Options to be printed +
    void Print(const Document& doc, std::ostream& output) {
        if (doc.GetRoot().IsInt()) {
            output << doc.GetRoot().AsInt();

        }
        else if (doc.GetRoot().IsDouble()) {
            output << doc.GetRoot().AsDouble();

        }
        else if (doc.GetRoot().IsBool()) {
            bool to_process = doc.GetRoot().AsBool();
            output << (to_process ? "true"s : "false"s);

        }
        else if (doc.GetRoot().IsNull()) {
            output << "null"s;

        }
        else if (doc.GetRoot().IsString()) {
            PrintString(doc, output);

        }
        else if (doc.GetRoot().IsArray()) {
            PrintArray(doc, output);

        }
        else {
            PrintMap(doc, output);
        }
    }
} // namespace json