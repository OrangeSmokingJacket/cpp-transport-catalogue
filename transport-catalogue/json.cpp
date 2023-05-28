#include "json.h"

using namespace std;

namespace json
{
    // ---------- Node ------------------

    Node LoadNode(istream& input);
    Node LoadString(istream& input)
    {
        using namespace literals;

        auto it = istreambuf_iterator<char>(input);
        auto end = istreambuf_iterator<char>();
        string s;
        while (true)
        {
            if (it == end)
                throw ParsingError("String parsing error");

            const char ch = *it;
            if (ch == '"')
            {
                // Встретили закрывающую кавычку
                it++;
                break;
            }
            else if (ch == '\\')
            {
                // Встретили начало escape-последовательности
                it++;
                if (it == end)
                    throw ParsingError("String parsing error");

                const char escaped_char = *(it);

                // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                switch (escaped_char)
                {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    // Встретили неизвестную escape-последовательность
                    throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                }
            }
            else if (ch == '\n' || ch == '\r')
            {
                // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                throw ParsingError("Unexpected end of line"s);
            }
            else
            {
                // Просто считываем очередной символ и помещаем его в результирующую строку
                s.push_back(ch);
            }
            it++;
        }

        return Node(move(s));
    }
    Node LoadNullBool(istream& input)
    {
        vector<string> results = { "null", "true", "false" };
        string result;
        char c = ' ';
        int index = -1;
        if (!(input >> c))
            throw ParsingError("");
        for (int i = 0; i < 3; i++)
        {
            if (c == results[i][0])
                index = i;
        }
        if (index == -1)
            throw ParsingError("");

        result.push_back(c);
        for (int i = 1; i < 4; i++)
        {
            if (!(input >> c))
                throw ParsingError("");

            if (c == results[index][i])
                result.push_back(c);
            else
                throw ParsingError("");
        }
        if (result == "fals")
        {
            if (!(input >> c))
                throw ParsingError("");

            if (c == 'e')
                result.push_back(c);
            else
                throw ParsingError("");
        }
        if (result == "null")
            return Node(nullptr);
        if (result == "true")
            return Node(true);
        if (result == "false")
            return Node(false);

        throw ParsingError("");
    }
    Node LoadArray(istream& input)
    {
        Array result;
        char c;
        while(input >> c && c != ']')
        {
            if (c != ',')
                input.putback(c);

            result.push_back(LoadNode(input));
        }
        if (c != ']')
            throw ParsingError("");
        else
            return Node(move(result));
    }
    Node LoadDict(istream& input)
    {
        Dict result;
        char c;
        while(input >> c && c != '}')
        {
            if (c == ',')
                input >> c;

            string key = LoadString(input).AsString();
            input >> c;
            result.insert({ move(key), LoadNode(input) });
        }
        if (c != '}')
            throw ParsingError("");
        else
            return Node(move(result));
    }
    Node LoadNumber(istream& input)
    {
        using namespace std::literals;

        std::string parsed_num;

        // Считывает в parsed_num очередной символ из input
        auto read_char = [&parsed_num, &input]
        {
            parsed_num += static_cast<char>(input.get());
            if (!input)
                throw json::ParsingError("Failed to read number from stream"s);
        };

        // Считывает одну или более цифр в parsed_num из input
        auto read_digits = [&input, read_char]
        {
            if (!std::isdigit(input.peek()))
                throw json::ParsingError("A digit is expected"s);

            while (std::isdigit(input.peek()))
            {
                read_char();
            }
        };

        if (input.peek() == '-')
            read_char();
        if (input.peek() == '0') // Парсим целую часть числа
            read_char(); // После 0 в JSON не могут идти другие цифры
        else
            read_digits();

        bool is_int = true;
        // Парсим дробную часть числа
        if (input.peek() == '.')
        {
            read_char();
            read_digits();
            is_int = false;
        }
        // Парсим экспоненциальную часть числа
        if (int ch = input.peek(); ch == 'e' || ch == 'E')
        {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-')
                read_char();

            read_digits();
            is_int = false;
        }

        try
        {
            if (is_int)
            {
                // Сначала пробуем преобразовать строку в int
                try
                {
                    return Node(std::stoi(parsed_num));
                }
                catch (...)
                {
                    // В случае неудачи, например, при переполнении,
                    // код ниже попробует преобразовать строку в double
                }
            }
            return Node(std::stod(parsed_num));
        }
        catch (...)
        {
            throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
        }
    }
    Node LoadNode(istream& input)
    {
        char c;
        input >> c;

        if (c == '[')
            return LoadArray(input);
        else if (c == '{')
            return LoadDict(input);
        else if (c == '"')
            return LoadString(input);

        input.putback(c);
        if (isdigit(c) || c == '-')
            return LoadNumber(input);
        else
            return LoadNullBool(input);

        throw ParsingError("");
    }

    Node::Node() : value(nullptr) {}
    Node::Node(nullptr_t ptr) : value(ptr) {}
    Node::Node(bool value_bool) : value(move(value_bool)) {}
    Node::Node(Array array) : value(move(array)) {}
    Node::Node(Dict map) : value(move(map)) {}
    Node::Node(int value_int) : value(move(value_int)) {}
    Node::Node(double value_double) : value(move(value_double)) {}
    Node::Node(string value_str) : value(move(value_str)) {}

    bool Node::IsNull() const
    {
        return holds_alternative<nullptr_t>(value);
    }
    bool Node::IsBool() const
    {
        return holds_alternative<bool>(value);
    }
    bool Node::IsArray() const
    {
        return holds_alternative<Array>(value);
    }
    bool Node::IsMap() const
    {
        return holds_alternative<Dict>(value);
    }
    bool Node::IsInt() const
    {
        return holds_alternative<int>(value);
    }
    bool Node::IsDouble() const
    {
        return holds_alternative<int>(value) || holds_alternative<double>(value);
    }
    bool Node::IsPureDouble() const
    {
        return holds_alternative<double>(value);
    }
    bool Node::IsString() const
    {
        return holds_alternative<string>(value);
    }

    bool Node::AsBool() const
    {
        if (!IsBool())
            throw logic_error("");

        return get<bool>(value);
    }
    const Array& Node::AsArray() const
    {
        if (!IsArray())
            throw logic_error("");

        return get<Array>(value);
    }
    const Dict& Node::AsMap() const
    {
        if (!IsMap())
            throw logic_error("");

        return std::get<Dict>(value);
    }
    int Node::AsInt() const
    {
        if (!IsInt())
            throw logic_error("");

        return get<int>(value);
    }
    double Node::AsDouble() const
    {
        if (IsPureDouble())
            return get<double>(value);
        if (IsInt())
            return static_cast<double>(get<int>(value));

            throw logic_error("");
    }
    const string& Node::AsString() const
    {
        if (!IsString())
            throw logic_error("");

        return get<string>(value);
    }

    size_t Node::GetVariantIndex() const
    {
        return value.index();
    }
    auto Node::GetVaruant() const
    {
        return value;
    }

    bool operator==(const Node& lhs, const Node& rhs)
    {
        if (lhs.GetVariantIndex() == rhs.GetVariantIndex())
        {
            if (lhs.GetVaruant() == rhs.GetVaruant())
                return true;
        }
        return false;
    }
    bool operator!=(const Node& lhs, const Node& rhs)
    {
        return !(lhs == rhs);
    }

    // ---------- Document --------------

    bool operator==(const Document& lhs, const Document& rhs)
    {
        return lhs.GetRoot() == rhs.GetRoot();
    }
    bool operator!=(const Document& lhs, const Document& rhs)
    {
        return !(lhs == rhs);
    }

    Document::Document(Node root) : root(move(root)) {}

    const Node& Document::GetRoot() const
    {
        return root;
    }
    Document Load(istream& input)
    {
        return Document{ LoadNode(input) };
    }

    // ---------- ValuePrinter ----------

    void ValuePrinter::operator()(std::nullptr_t) const
    {
        out << "null";
    }
    void ValuePrinter::operator()(Array array) const
    {
        out << "[";
        bool first = true;
        for (const Node& node : array)
        {
            if (!first)
                out << ", ";

            out << node;
            first = false;
        }
        out << "]";
    }
    void ValuePrinter::operator()(Dict dict) const
    {
        out << "{";
        bool first = true;
        for (const auto& [key, value] : dict)
        {
            if (!first)
                out << ", ";

            out << "\"" << key << "\" : " << value;
            first = false;
        }
        out << "}";
    }
    void ValuePrinter::operator()(bool b) const
    {
        out << std::boolalpha << b;
    }
    void ValuePrinter::operator()(int i) const
    {
        out << i;
    }
    void ValuePrinter::operator()(double d) const
    {
        out << d;
    }
    void ValuePrinter::operator()(std::string str) const
    {
        out << "\"" << ConvertOutputString(str) << "\"";
    }

    std::string ValuePrinter::ConvertOutputString(const std::string& str) const
    {
        std::string result = str;
        size_t pos = result.find('\\');
        while (pos != std::string::npos)
        {
            result.replace(pos, 1, "\\\\");
            pos = result.find('\\', pos + 2);
        }
        pos = result.find('\r');
        while (pos != std::string::npos)
        {
            result.replace(pos, 1, "\\r");
            pos = result.find('\r', pos + 1);
        }
        pos = result.find('\n');
        while (pos != std::string::npos)
        {
            result.replace(pos, 1, "\\n");
            pos = result.find('\n', pos + 1);
        }
        pos = result.find("\"");
        while (pos != std::string::npos)
        {
            result.replace(pos, 1, "\\\"");
            pos = result.find("\"", pos + 2);
        }

        return result;
    }

    ostream& operator<<(ostream& out, const Node& node)
    {
        visit(ValuePrinter{ out }, node.GetVaruant());
        return out;
    }
    void Print(const Document& doc, ostream& output)
    {
        output << doc.GetRoot();
    }
}