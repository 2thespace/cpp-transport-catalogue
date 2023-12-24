#include "json.h"

using namespace std;

std::string LoadLiteral(std::istream& input) {
    std::string s;
    while (std::isalpha(input.peek())) {
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}
namespace json {

    namespace {


        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            if (input.peek() == -1) throw ParsingError("Array parsing error");
            for (char c; input >> c && c != ']';) {
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            return Node(move(result));
        }

        using Number = std::variant<int, double>;

        Node LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return  std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return  std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
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
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadDict(istream& input) {
            Dict result;
            if (input.peek() == -1) throw ParsingError("Array parsing error");

            for (char c; input >> c && c != '}';) {
                if (c == ',') {
                    input >> c;
                }

                string key = LoadString(input);
                input >> c;
                result.insert({ std::move(key), LoadNode(input) });
            }

            return Node(std::move(result));
        }

        Node LoadNull(istream& input)
        {
            string line = LoadLiteral(input);
            
            if (line == "null") {
                return Node{};
            }
            else {
                throw ParsingError("Null parsing error");
            }


        }
        Node LoadBool(istream& input)
        {
            string line = LoadLiteral(input);
            if (line == "true") {
                return Node{ true };
            }
            else if (line == "false") {
                return Node{ false };
            }
            else {
                throw ParsingError("Null parsing error");
            }
        }
        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (c == 'f' || c == 't') {
                input.putback(c);
                return LoadBool(input);
            }
            else if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else {
                input.putback(c);
                return LoadNumber(input);
            }
        }

    }  // namespace

    Node::Node(std::nullptr_t) :node_(nullptr) {}
    Node::Node(Array array)
        : node_(move(array)) {
    }

    Node::Node(Dict map)
        : node_(move(map)) {
    }

    Node::Node(int value)
        : node_(value) {
    }
    Node::Node(double value)
        : node_(value) {
    }
    Node::Node(string value)
        : node_(move(value)) {
    }
    Node::Node(bool value)
        :node_(value) {
    }
    bool Node::IsInt() const
    {
        return std::holds_alternative<int>(node_);
    }
    bool Node::IsDouble() const
    {
        return  IsInt() || IsPureDouble();
    }
    bool Node::IsPureDouble() const
    {
        return std::holds_alternative<double>(node_);
    }
    bool Node::IsBool() const
    {
        return std::holds_alternative<bool>(node_);
    }
    bool Node::IsString() const
    {
        return std::holds_alternative<std::string>(node_);
    }
    bool Node::IsNull() const
    {
        return std::holds_alternative<std::nullptr_t>(node_);
    }
    bool Node::IsArray() const
    {
        return std::holds_alternative<Array>(node_);
    }
    bool Node::IsMap() const
    {
        return std::holds_alternative<Dict>(node_);
    }
    const Array& Node::AsArray() const
    {
        if (!IsArray()) {
            throw std::logic_error("Is not array");
        }
        return std::get<Array>(node_);
    }
    const Dict& Node::AsMap() const
    {
        if (!IsMap()) {
            throw std::logic_error("Is not dict");
        }
        return std::get<Dict>(node_);
    }
    int Node::AsInt() const
    {
        if (!IsInt()) {
            throw std::logic_error("Is not dict");
        }
        return std::get<int>(node_);
    }
    const std::string& Node::AsString() const
    {
        if (!IsString()) {
            throw std::logic_error("Is not string");
        }
        return std::get<std::string>(node_);
    }
    double Node::AsDouble() const {
        using namespace std::literals;
        if (!IsDouble()) {
            throw std::logic_error("Not a double"s);
        }
        return IsPureDouble() ? std::get<double>(node_) : AsInt();
    }
    bool Node::AsBool() const
    {
        if (!IsBool()) {
            throw std::logic_error("Is not bool");
        }
        return std::get<bool>(node_);
    }
    bool Node::operator ==(const Node& right) const
    {
        return this->node_ == right.node_;
    }
    bool Node::operator !=(const Node& right) const
    {
        return this->node_ != right.node_;
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }
    bool Document::operator==(const Document& rhs) const {
        return root_ == rhs.root_;
    }

    bool Document::operator!=(const Document& rhs) const {
        return !(root_ == rhs.root_);
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }


    void ValuePrinter::operator()(int value) {
        out << value;
    }
    void ValuePrinter::operator()(double value) {
        out << value;
    }
    void ValuePrinter::operator()(std::string value) {
        out << "\""sv;
        for (const char& c : value) {
            if (c == '\n') {
                out << "\\n"sv;
                continue;
            }
            if (c == '\r') {
                out << "\\r"sv;
                continue;
            }
            if (c == '\"') out << "\\"sv;
            if (c == '\t') {
                out << "\\t"sv;
                continue;
            }
            if (c == '\\') out << "\\"sv;
            out << c;
        }
        out << "\""sv;
    }
    void ValuePrinter::operator()(bool value) {
        out << std::boolalpha << value;
    }
    void ValuePrinter::operator()(std::nullptr_t) {
        out << "null"sv;
    }

    void ValuePrinter::operator()(Array values) {
        bool is_first = true;
        out << "[";
        for (const auto& node : values)
        {
            if (is_first) {
                is_first = false;
            }
            else {
                out << ", ";
            }
            PrintNode(node, out);
        }
        out << "]";
    }

    void ValuePrinter::operator()(Dict dict) {
        bool is_first = true;
        out << "{";
        for (auto& [key, value] : dict)
        {
            if (is_first == true) {
                is_first = false;
            }
            else {
                out << ", ";

            }

            out << "\"" << key << "\": ";
            PrintNode(value, out);
        }
        out << "}";
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(ValuePrinter{ out }, node.GetValue());
    }
    void Print(const Document& doc, std::ostream& output) {
        auto node = doc.GetRoot();
        PrintNode(node, output);

        // Реализуйте функцию самостоятельно
    }

}  // namespace json