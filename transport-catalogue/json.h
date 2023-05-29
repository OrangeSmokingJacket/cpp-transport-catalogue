#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json
{
    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    class Node : public std::variant<std::nullptr_t, bool, Array, Dict, int, double, std::string>
    {
        using variant = variant;
    public:
        Node();
        Node(std::nullptr_t ptr);
        Node(bool value_bool);
        Node(Array array);
        Node(Dict map);
        Node(int value_int);
        Node(double value_double);
        Node(std::string value_str);

        bool IsNull() const;
        bool IsBool() const;
        bool IsArray() const;
        bool IsMap() const;
        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsString() const;

        bool AsBool() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;

        size_t GetVariantIndex() const;
        auto GetVaruant() const;
    };

    bool operator==(const Node& lhs, const Node& rhs);
    bool operator!=(const Node& lhs, const Node& rhs);

    class Document
    {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root;
    };

    bool operator==(const Document& lhs, const Document& rhs);
    bool operator!=(const Document& lhs, const Document& rhs);

    Document Load(std::istream& input);

    std::ostream& operator<<(std::ostream& out, const Node& node);
    struct ValuePrinter
    {
        //using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
        std::ostream& out;
        void operator()(std::nullptr_t) const;
        void operator()(Array array) const;
        void operator()(Dict dict) const;
        void operator()(bool b) const;
        void operator()(int i) const;
        void operator()(double d) const;
        void operator()(std::string str) const;
    private:

        std::string ConvertOutputString(const std::string& str) const;
    };
    void Print(const Document& doc, std::ostream& output);
}