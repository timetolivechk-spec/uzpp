#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <optional>
#include <memory>
#include <cctype>

namespace uzpp::Json {

struct Qiymat;

using JsonMassiv  = std::vector<std::shared_ptr<Qiymat>>;
using JsonObyekt  = std::unordered_map<std::string, std::shared_ptr<Qiymat>>;
using JsonVariant = std::variant<std::nullptr_t, bool, double, std::string, JsonMassiv, JsonObyekt>;

struct Qiymat {
    JsonVariant malumot;

    Qiymat()                              : malumot(nullptr) {}
    explicit Qiymat(std::nullptr_t)       : malumot(nullptr) {}
    explicit Qiymat(bool b)               : malumot(b) {}
    explicit Qiymat(double d)             : malumot(d) {}
    explicit Qiymat(int i)                : malumot(static_cast<double>(i)) {}
    explicit Qiymat(long long l)          : malumot(static_cast<double>(l)) {}
    explicit Qiymat(const std::string& s) : malumot(s) {}
    explicit Qiymat(std::string&& s)      : malumot(std::move(s)) {}
    explicit Qiymat(JsonMassiv arr)       : malumot(std::move(arr)) {}
    explicit Qiymat(JsonObyekt obj)       : malumot(std::move(obj)) {}

    [[nodiscard]] bool nullmi()    const noexcept { return std::holds_alternative<std::nullptr_t>(malumot); }
    [[nodiscard]] bool mantiqmi()  const noexcept { return std::holds_alternative<bool>(malumot); }
    [[nodiscard]] bool sonmi()     const noexcept { return std::holds_alternative<double>(malumot); }
    [[nodiscard]] bool matnmi()    const noexcept { return std::holds_alternative<std::string>(malumot); }
    [[nodiscard]] bool massivmi()  const noexcept { return std::holds_alternative<JsonMassiv>(malumot); }
    [[nodiscard]] bool obyektmi()  const noexcept { return std::holds_alternative<JsonObyekt>(malumot); }

    [[nodiscard]] bool mantiq() const {
        if (!mantiqmi()) throw std::runtime_error("JSON qiymat bool emas");
        return std::get<bool>(malumot);
    }
    [[nodiscard]] double son() const {
        if (!sonmi()) throw std::runtime_error("JSON qiymat son emas");
        return std::get<double>(malumot);
    }
    [[nodiscard]] int butun() const { return static_cast<int>(son()); }
    [[nodiscard]] const std::string& matn() const {
        if (!matnmi()) throw std::runtime_error("JSON qiymat matn emas");
        return std::get<std::string>(malumot);
    }
    [[nodiscard]] const JsonMassiv& massiv() const {
        if (!massivmi()) throw std::runtime_error("JSON qiymat massiv emas");
        return std::get<JsonMassiv>(malumot);
    }
    [[nodiscard]] const JsonObyekt& obyekt() const {
        if (!obyektmi()) throw std::runtime_error("JSON qiymat ob'yekt emas");
        return std::get<JsonObyekt>(malumot);
    }

    [[nodiscard]] std::shared_ptr<Qiymat> operator[](const std::string& key) const {
        if (!obyektmi()) throw std::runtime_error("JSON qiymat ob'yekt emas");
        const auto& o = std::get<JsonObyekt>(malumot);
        auto it = o.find(key);
        return it == o.end() ? nullptr : it->second;
    }

    [[nodiscard]] std::shared_ptr<Qiymat> operator[](std::size_t idx) const {
        if (!massivmi()) throw std::runtime_error("JSON qiymat massiv emas");
        return std::get<JsonMassiv>(malumot).at(idx);
    }

    [[nodiscard]] bool bor(const std::string& key) const {
        if (!obyektmi()) return false;
        return std::get<JsonObyekt>(malumot).count(key) > 0;
    }

    [[nodiscard]] std::optional<std::shared_ptr<Qiymat>> olish(const std::string& key) const {
        if (!obyektmi()) return std::nullopt;
        const auto& o = std::get<JsonObyekt>(malumot);
        auto it = o.find(key);
        if (it == o.end()) return std::nullopt;
        return it->second;
    }

    // Set a field (for object building)
    void set(const std::string& key, std::shared_ptr<Qiymat> val) {
        if (!obyektmi()) throw std::runtime_error("JSON qiymat ob'yekt emas");
        std::get<JsonObyekt>(malumot)[key] = std::move(val);
    }

    void qosh(std::shared_ptr<Qiymat> val) {
        if (!massivmi()) throw std::runtime_error("JSON qiymat massiv emas");
        std::get<JsonMassiv>(malumot).push_back(std::move(val));
    }

    explicit operator bool() const noexcept { return !nullmi(); }
};

// ===== SERIALIZER =====

namespace detail {

inline std::string escape(const std::string& s) {
    std::ostringstream ss;
    ss << '"';
    for (unsigned char c : s) {
        switch (c) {
            case '"':  ss << "\\\""; break;
            case '\\': ss << "\\\\"; break;
            case '\b': ss << "\\b";  break;
            case '\f': ss << "\\f";  break;
            case '\n': ss << "\\n";  break;
            case '\r': ss << "\\r";  break;
            case '\t': ss << "\\t";  break;
            default:
                if (c < 0x20) ss << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                else           ss << static_cast<char>(c);
        }
    }
    ss << '"';
    return ss.str();
}

inline std::string serialize(const Qiymat& q, int indent, int level) {
    const std::string pad(static_cast<std::size_t>(indent > 0 ? indent * level : 0), ' ');
    const std::string childPad(static_cast<std::size_t>(indent > 0 ? indent * (level + 1) : 0), ' ');
    const std::string sep = indent > 0 ? "\n" : "";
    const std::string colSep = indent > 0 ? ": " : ":";

    if (q.nullmi())   return "null";
    if (q.mantiqmi()) return q.mantiq() ? "true" : "false";
    if (q.sonmi()) {
        std::ostringstream ss;
        double d = q.son();
        if (d == static_cast<long long>(d)) ss << static_cast<long long>(d);
        else ss << d;
        return ss.str();
    }
    if (q.matnmi()) return escape(q.matn());

    if (q.massivmi()) {
        const auto& arr = q.massiv();
        if (arr.empty()) return "[]";
        std::ostringstream ss;
        ss << "[" << sep;
        for (std::size_t i = 0; i < arr.size(); ++i) {
            ss << childPad << serialize(*arr[i], indent, level + 1);
            if (i + 1 < arr.size()) ss << ",";
            ss << sep;
        }
        ss << pad << "]";
        return ss.str();
    }

    if (q.obyektmi()) {
        const auto& obj = q.obyekt();
        if (obj.empty()) return "{}";
        std::ostringstream ss;
        ss << "{" << sep;
        bool first = true;
        for (const auto& [k, v] : obj) {
            if (!first) ss << "," << sep;
            first = false;
            ss << childPad << escape(k) << colSep << serialize(*v, indent, level + 1);
        }
        ss << sep << pad << "}";
        return ss.str();
    }
    return "null";
}

} // namespace detail

[[nodiscard]] inline std::string chiqarish(const Qiymat& q, int indent = 0) {
    return detail::serialize(q, indent, 0);
}

[[nodiscard]] inline std::string chiqarish(const std::shared_ptr<Qiymat>& q, int indent = 0) {
    if (!q) return "null";
    return detail::serialize(*q, indent, 0);
}

// ===== PARSER =====

namespace detail {

struct ParseState {
    const std::string& src;
    std::size_t pos = 0;

    [[nodiscard]] char cur() const { return pos < src.size() ? src[pos] : '\0'; }
    void next() { if (pos < src.size()) ++pos; }
    void skip() { while (pos < src.size() && (cur() == ' ' || cur() == '\t' || cur() == '\n' || cur() == '\r')) ++pos; }
    [[nodiscard]] bool done() const { return pos >= src.size(); }
};

std::shared_ptr<Qiymat> parseVal(ParseState& p);

inline std::string parseStr(ParseState& p) {
    p.next(); // skip '"'
    std::string r;
    while (!p.done() && p.cur() != '"') {
        if (p.cur() == '\\') {
            p.next();
            switch (p.cur()) {
                case '"':  r += '"';  break;
                case '\\': r += '\\'; break;
                case '/':  r += '/';  break;
                case 'b':  r += '\b'; break;
                case 'f':  r += '\f'; break;
                case 'n':  r += '\n'; break;
                case 'r':  r += '\r'; break;
                case 't':  r += '\t'; break;
                default:   r += p.cur();
            }
        } else {
            r += p.cur();
        }
        p.next();
    }
    p.next(); // closing '"'
    return r;
}

inline double parseNum(ParseState& p) {
    std::string n;
    if (p.cur() == '-') { n += '-'; p.next(); }
    while (std::isdigit(static_cast<unsigned char>(p.cur()))) { n += p.cur(); p.next(); }
    if (p.cur() == '.') { n += '.'; p.next(); while (std::isdigit(static_cast<unsigned char>(p.cur()))) { n += p.cur(); p.next(); } }
    if (p.cur() == 'e' || p.cur() == 'E') {
        n += p.cur(); p.next();
        if (p.cur() == '+' || p.cur() == '-') { n += p.cur(); p.next(); }
        while (std::isdigit(static_cast<unsigned char>(p.cur()))) { n += p.cur(); p.next(); }
    }
    return std::stod(n);
}

inline std::shared_ptr<Qiymat> parseArr(ParseState& p) {
    p.next(); // '['
    JsonMassiv arr;
    p.skip();
    if (p.cur() == ']') { p.next(); return std::make_shared<Qiymat>(std::move(arr)); }
    while (true) {
        p.skip();
        arr.push_back(parseVal(p));
        p.skip();
        if (p.cur() == ']') { p.next(); break; }
        if (p.cur() != ',') throw std::runtime_error("JSON: ',' kutilgan massivda");
        p.next();
    }
    return std::make_shared<Qiymat>(std::move(arr));
}

inline std::shared_ptr<Qiymat> parseObj(ParseState& p) {
    p.next(); // '{'
    JsonObyekt obj;
    p.skip();
    if (p.cur() == '}') { p.next(); return std::make_shared<Qiymat>(std::move(obj)); }
    while (true) {
        p.skip();
        if (p.cur() != '"') throw std::runtime_error("JSON: kalit matn bo'lishi kerak");
        auto key = parseStr(p);
        p.skip();
        if (p.cur() != ':') throw std::runtime_error("JSON: ':' kutilgan");
        p.next();
        p.skip();
        obj[key] = parseVal(p);
        p.skip();
        if (p.cur() == '}') { p.next(); break; }
        if (p.cur() != ',') throw std::runtime_error("JSON: ',' kutilgan ob'yektda");
        p.next();
    }
    return std::make_shared<Qiymat>(std::move(obj));
}

inline std::shared_ptr<Qiymat> parseVal(ParseState& p) {
    p.skip();
    char c = p.cur();
    if (c == '"') return std::make_shared<Qiymat>(parseStr(p));
    if (c == '[') return parseArr(p);
    if (c == '{') return parseObj(p);
    if (c == 't') { p.pos += 4; return std::make_shared<Qiymat>(true); }
    if (c == 'f') { p.pos += 5; return std::make_shared<Qiymat>(false); }
    if (c == 'n') { p.pos += 4; return std::make_shared<Qiymat>(nullptr); }
    if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) return std::make_shared<Qiymat>(parseNum(p));
    throw std::runtime_error(std::string("JSON: kutilmagan belgi '") + c + "'");
}

} // namespace detail

[[nodiscard]] inline std::shared_ptr<Qiymat> tahlil(const std::string& text) {
    detail::ParseState p{text, 0};
    auto result = detail::parseVal(p);
    p.skip();
    if (!p.done()) throw std::runtime_error("JSON: ortiqcha ma'lumot topildi");
    return result;
}

// Factory helpers
[[nodiscard]] inline std::shared_ptr<Qiymat> yaratObyekt() {
    return std::make_shared<Qiymat>(JsonObyekt{});
}
[[nodiscard]] inline std::shared_ptr<Qiymat> yaratMassiv() {
    return std::make_shared<Qiymat>(JsonMassiv{});
}
[[nodiscard]] inline std::shared_ptr<Qiymat> yaratMatn(const std::string& s) {
    return std::make_shared<Qiymat>(s);
}
[[nodiscard]] inline std::shared_ptr<Qiymat> yaratSon(double d) {
    return std::make_shared<Qiymat>(d);
}
[[nodiscard]] inline std::shared_ptr<Qiymat> yaratMantiq(bool b) {
    return std::make_shared<Qiymat>(b);
}
[[nodiscard]] inline std::shared_ptr<Qiymat> yaratNull() {
    return std::make_shared<Qiymat>(nullptr);
}

} // namespace uzpp::Json
