#pragma once

#include "types.h"

#include <span>
#include <variant>
#include <string>
#include <cstdint>
#include <utility>
#include <optional>
#include <type_traits>
#include <memory_resource>

namespace vsc {
    struct Decimal {
        using allocator_type = std::pmr::polymorphic_allocator<char>;

        Text repr;

        explicit Decimal(allocator_type alloc = {}) noexcept
            : repr(alloc)
        { }

        explicit Decimal(Text repr, allocator_type alloc)
            : repr(std::move(repr), alloc)
        { }

        Decimal(const Decimal& other, allocator_type alloc)
            : repr(other.repr, alloc)
        { }

        Decimal(Decimal&& other, allocator_type alloc)
            : repr(std::move(other.repr), alloc)
        { }

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

        using allocator_type = std::pmr::polymorphic_allocator<char>;

        explicit Value(Var v) noexcept
            : v_(std::move(v))
        { }

        explicit Value(Var v, allocator_type alloc) {
            v_ = std::visit([alloc](auto&& val) -> Var {
                using T = std::remove_cvref_t<decltype(val)>;

                if constexpr (std::is_same_v<T, Text> || std::is_same_v<T, Blob> || std::is_same_v<T, Decimal>)
                    return Var(std::in_place_type<T>, std::move(val), alloc);
                else
                    return Var(std::in_place_type<T>, std::move(val));
            }, std::move(v));
        }

        Value(const Value& other, allocator_type alloc) {
            v_ = std::visit([alloc](const auto& val) -> Var {
                using T = std::remove_cvref_t<decltype(val)>;

                if constexpr (std::is_same_v<T, Text> || std::is_same_v<T, Blob> || std::is_same_v<T, Decimal>)
                    return Var(std::in_place_type<T>, val, alloc);
                else
                    return Var(std::in_place_type<T>, val);
            }, other.v_);
        }

        Value(Value&& other, allocator_type alloc) noexcept {
            v_ = std::visit([alloc](auto&& val) -> Var {
                using T = std::remove_cvref_t<decltype(val)>;

                if constexpr (std::is_same_v<T, Text> || std::is_same_v<T, Blob> || std::is_same_v<T, Decimal>)
                    return Var(std::in_place_type<T>, std::move(val), alloc);
                else
                    return Var(std::in_place_type<T>, std::move(val));
            }, std::move(other.v_));
        }

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
                [](const Text& val)    -> std::string { return { val.data(), val.size() }; },
                [](const Blob& val)    -> std::string { return "[Blob: " + std::to_string(val.size()) + " bytes]";},
                [](const Decimal& val) -> std::string { return { val.repr.data(), val.repr.size() }; }
                              }, v_);
        }

        bool operator==(const Value& other) const = default;

        private:
            Var v_;

            template <typename T> std::optional<T> as() const {
                if (const T* p = std::get_if<T>(&v_))
                    return *p;

                [[unlikely]] return std::nullopt;
            }
    };


    [[nodiscard]] inline Value makeNull() noexcept {
        return Value {
            Value::Var{std::in_place_type<std::monostate>}
        };
    }

    [[nodiscard]] inline Value makeInt(std::int64_t i) noexcept {
        return Value {
            Value::Var{std::in_place_type<std::int64_t>, i}
        };
    }

    [[nodiscard]] inline Value makeReal(double d) noexcept {
        return Value {
            Value::Var{std::in_place_type<double>, d}
        };
    }

    [[nodiscard]] inline Value makeBool(bool b) noexcept {
        return Value {
            Value::Var{std::in_place_type<bool>, b}
        };
    }

    [[nodiscard]] inline Value makeText(Text::allocator_type alloc, std::string_view src) {
        Text t(alloc);
        t.assign(src.begin(), src.end());
        return Value{ Value::Var{ std::in_place_type<Text>, std::move(t), alloc } };
    }

    [[nodiscard]] inline Value makeBlob(Text::allocator_type alloc, std::span<const std::uint8_t> src) {
        Blob b(alloc);
        b.assign(src.begin(), src.end());
        return Value { Value::Var{std::in_place_type<Blob>, std::move(b), alloc} };
    }

    [[nodiscard]] inline Value makeDecimal(Text::allocator_type alloc, std::string_view src) {
        Decimal d(alloc);
        d.repr.assign(src.begin(), src.end());
        return Value { Value::Var{std::in_place_type<Decimal>, std::move(d), alloc} };
    }
}

template <typename Alloc>
struct std::uses_allocator<vsc::Decimal, Alloc> : std::true_type {};

template <typename Alloc>
struct std::uses_allocator<vsc::Value, Alloc> : std::true_type {};