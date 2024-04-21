#include <iostream>
#include <vector>
#include <memory>
#include <variant>
#include <unordered_map>

// (1 + 2) * 3 / 4 + 5 * (6 - 7)

// 1. Токенизация
// 2. Парсер (построение дерева разбора выражения)

// +, -, *, /, %

struct OpeningBracket {};

struct ClosingBracket {};

struct Number {
    int value;
};

struct UnknownToken {
    std::string value;
};

struct MinToken {};

struct MaxToken{};

struct AbsToken {};

struct Plus {};

struct Minus {};

struct Multiply {};

struct Modulo {};

struct Divide {};

struct Sqr {};

using Token = std::variant<OpeningBracket, ClosingBracket, Number, UnknownToken, MinToken, MaxToken, AbsToken, Plus, Minus, Sqr, Multiply, Divide, Modulo>;

// 1234

const std::unordered_map<char, Token> kSymbol2Token{
        {'+', Token(Plus{})}, {'-', Token(Minus{})}, {'*', Token(Multiply{})}, {'/', Token(Divide{})}, {'%', Token(Modulo{})}};
const std::unordered_map<std::string, Token> kString2Token{
        {"max", Token(MaxToken{})}, {"min", Token(MinToken{})}, {"abs", Token(AbsToken{})}, {"sqr", Token(Sqr{})}};

int ToDigit(unsigned char symbol) {
    return symbol - '0';
}

Number ParseNumber(const std::string& input, size_t& pos) {
    int value = 0;
    auto symbol = static_cast<unsigned char>(input[pos]);
    while (std::isdigit(symbol)) {
        value = value * 10 + ToDigit(symbol);
        if (pos == input.size() - 1) {
            break;
        }
        symbol = static_cast<unsigned char>(input[++pos]);
    }
    return Number{value};
}

Token ParseName(const std::string& input, size_t& pos) {
    std::string name;
    while (pos < input.size() && std::isalpha(input[pos])) {
        name.push_back(input[pos]);
        ++pos;
    }

    if (!name.empty()) {
        if (auto it = kString2Token.find(name); it != kString2Token.end()) {
            return it->second;
        } else {
            return UnknownToken{name};
        }
    } else {
        return UnknownToken{name};
    }
}

std::vector<Token> Tokenize(const std::string& input) {
    std::vector<Token> tokens;
    const size_t size = input.size();
    size_t pos = 0;
    while (pos < size) {
        const auto symbol = static_cast<unsigned char>(input[pos]);
        if (std::isspace(symbol)) {
            ++pos;
        } else if (std::isdigit(symbol)) {
            tokens.emplace_back(ParseNumber(input, pos));
        } else if (std::isalpha(symbol)) {
            tokens.emplace_back(ParseName(input, pos));
        } else if (auto it = kSymbol2Token.find(symbol); it != kSymbol2Token.end()) {
            tokens.emplace_back(it->second);
        }
    }
    return tokens;
}