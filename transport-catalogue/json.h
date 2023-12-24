#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {

    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, int, double, std::string, bool>;
        /* Реализуйте Node, используя std::variant */
        Node() = default;
        Node(std::nullptr_t);
        Node(Array array);
        Node(Dict map);
        Node(int value);
        Node(double value);
        Node(std::string value);
        Node(bool value);

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        double AsDouble() const;
        bool AsBool() const;
        bool operator ==(const Node& right) const;
        bool operator !=(const Node& right) const;

        // ...
        const Value& GetValue() const { return node_; }
    private:
        Value node_;
    };

    class Document {
    public:
        explicit Document(Node root);
        Document() = default;
        const Node& GetRoot() const;
        bool operator==(const Document& rhs) const;
        bool operator!=(const Document& rhs) const;
    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);
    void PrintNode(const Node& node, std::ostream& out);
    struct ValuePrinter {
        std::ostream& out;
        void operator()(std::nullptr_t);
        void operator()(std::string value);
        void operator()(int value);
        void operator()(double value);
        void operator()(bool value);
        void operator()(Array array);
        void operator()(Dict dict);
    };
}  // namespace json