#pragma once

#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {
// ------------------------------------ [JSON] Definition -----------------------------
//                                                                                    +
//                                                                                    + ------
// ------------------------------------------------------------------------------------ Node +
    class Node;

    using Array = std::vector<Node>;
    using Dict = std::map<std::string, Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final {
    public:
        bool operator==(const Node& other) const;
        bool operator!=(const Node& other) const;

        Node() = default;
        Node(int value) noexcept;
        Node(double value) noexcept;
        Node(bool boolean) noexcept;
        Node(std::string str);
        Node(std::nullptr_t null) noexcept;
        Node(Array array);
        Node(Dict map);

        bool IsInt() const noexcept;
        bool IsDouble() const noexcept;
        bool IsPureDouble() const noexcept;
        bool IsBool() const noexcept;
        bool IsString() const noexcept;
        bool IsNull() const noexcept;
        bool IsArray() const noexcept;
        bool IsMap() const noexcept;

        int AsInt() const;
        double AsDouble() const;
        bool AsBool() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

    private:
        template <class Type>
        const Type* Getter() const;

        std::variant<std::nullptr_t, int, double, bool, std::string, Array, Dict> json_lib_;
    };

    // Я вложил в этот Getter душу. Пожалуйста, позвольте ему жить xD
    template <class Type>
    const Type* Node::Getter() const {
        if (const Type* to_return = std::get_if<Type>(&json_lib_); to_return != nullptr) {
            return to_return;
        }

        throw std::logic_error("Wrong type has been requested.");
    }

// --------------------------- [Storage & Main Loader] Definition ---------------------
//                                                                                    +
//                                                                                    + -----------------------
// ------------------------------------------------------------------------------------ Storage & Main Loader +
    class Document final {
    public:
        bool operator==(const Document& other) const;
        bool operator!=(const Document& other) const;

        explicit Document(Node root);
        const Node& GetRoot() const;

    private:
        Node root_;
    };

    Document Load(std::istream& input);

// --------------------------------- [Printers] Definition ----------------------------
//                                                                                    +
//                                                                                    + ----------
// ------------------------------------------------------------------------------------ Printers +
    void PrintString(const Document& doc, std::ostream& output);
    void PrintArray(const Document& doc, std::ostream& output);
    void PrintMap(const Document& doc, std::ostream& output);

    void Print(const Document& doc, std::ostream& output);
} // namespace json