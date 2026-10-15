#pragma once

#include <variant>
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>

namespace vsc {

    using Blob = std::vector<std::uint8_t>;
    using Text = std::string;

    struct Decimal {
        Text repr;

        explicit Decimal(Text repr)
            : repr(std::move(repr))
        {}

        /* Сравнение по строковому представлению */
        bool operator==(const Decimal& other) const {
            return this->repr == other.repr;
        }
    };

    enum class ValueType {
        null,
        integer,
        real,
        boolean,
        text,
        blob,
        decimal
    };

    namespace detail {
        template <typename... Ts>
        struct overloaded : Ts... {
            using Ts::operator()...;
        };

        template <typename... Ts>
        overloaded(Ts...) -> overloaded<Ts...>;
    }

    template <typename T> struct ValueTypeOf;
        template <> struct ValueTypeOf<std::monostate> { static constexpr ValueType value = ValueType::null; };
        template <> struct ValueTypeOf<std::int64_t>   { static constexpr ValueType value = ValueType::integer; };
        template <> struct ValueTypeOf<double>         { static constexpr ValueType value = ValueType::real; };
        template <> struct ValueTypeOf<bool>           { static constexpr ValueType value = ValueType::boolean; };
        template <> struct ValueTypeOf<Text>           { static constexpr ValueType value = ValueType::text; };
        template <> struct ValueTypeOf<Blob>           { static constexpr ValueType value = ValueType::blob; };
        template <> struct ValueTypeOf<Decimal>        { static constexpr ValueType value = ValueType::decimal; };

    struct Value {
        using Var = std::variant<
        std::monostate,           // NULL
        std::int64_t,             // PK, count
        double,                   // REAL / DOUBLE / FLOAT
        bool,                     // BOOLEAN / YESNO
        Text,                     // TEXT / VARCHAR
        Blob,                     // BLOB
        Decimal                   // DECIMAL
        >;

        explicit Value(Var v)
            : v_(std::move(v))
        {}

        ValueType type() const {
            return std::visit([](const auto& x) {
                using T = std::remove_cvref_t<decltype(x)>;
                return ValueTypeOf<T>::value;
            }, v_);
        }

        bool isNull() const {
            return std::holds_alternative<std::monostate>(v_);
        }

        std::optional<std::int64_t> asInt64()   const { return as<std::int64_t>(); }
        std::optional<double>       asDouble()  const { return as<double>(); }
        std::optional<bool>         asBool()    const { return as<bool>();}
        std::optional<Text>         asString()  const { return as<Text>(); }
        std::optional<Blob>         asBlob()    const { return as<Blob>(); }
        std::optional<Decimal>      asDecimal() const { return as<Decimal>(); }

        std::string toString() const {
            return std::visit(detail::overloaded {
                [](std::monostate)     -> std::string { return ""; },
                [](std::int64_t val)   -> std::string { return std::to_string(val); },
                [](double val)         -> std::string { return std::to_string(val); },
                [](bool val)           -> std::string { return val ? "true" : "false"; },
                [](const Text& val)    -> std::string { return val; },
                [](const Blob& val)    -> std::string { return "[Blob: " + std::to_string(val.size()) + " bytes]";},
                [](const Decimal& val) -> std::string { return val.repr; }
                              }, v_);
        }

        bool operator==(const Value& other) const = default;

        private:
            Var v_;

            template <typename T> std::optional<T> as() const {
                if (const T* p = std::get_if<T>(&v_))
                    return *p;

                return std::nullopt;
            }
    };


    inline Value makeNull() {
        return Value {
            Value::Var{std::in_place_type<std::monostate>}
        };
    }

    inline Value makeInt(std::int64_t i) {
        return Value {
            Value::Var{std::in_place_type<std::int64_t>, i}
        };
    }

    inline Value makeReal(double d) {
        return Value {
            Value::Var{std::in_place_type<double>, d}
        };
    }

    inline Value makeBool(bool b) {
        return Value {
            Value::Var{std::in_place_type<bool>, b}
        };
    }

    inline Value makeText(Text s) {
        return Value {
            Value::Var{std::in_place_type<Text>, std::move(s)}
        };
    }

    inline Value makeBlob(Blob bytes) {
        return Value {
            Value::Var{std::in_place_type<Blob>, std::move(bytes)}
        };
    }

    inline Value makeDecimal(Text repr) {
        return Value {
            Value::Var{std::in_place_type<Decimal>, std::move(repr)}
        };
    }
}