#pragma once

#include <variant>
#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace vsc {

    using Blob = std::vector<std::uint8_t>;
    using Text = std::string;

    enum class ValueType {
        null,
        integer,
        real,
        boolean,
        text,
        blob
    };

    struct Value {
        using Var = std::variant<
        std::monostate,           // NULL
        std::int64_t,             // PK, count
        double,                   // REAL / DOUBLE / FLOAT
        bool,                     // BOOLEAN / YESNO
        Text,                     // TEXT / VARCHAR
        Blob                      // BLOB
        >;


        explicit Value(Var v) : v_(std::move(v)) { }



        vsc::ValueType type() const {
            switch(v_.index()) {
            case 0: return ValueType::null;
            case 1: return ValueType::integer;
            case 2: return ValueType::real;
            case 3: return ValueType::boolean;
            case 4: return ValueType::text;
            case 5: return ValueType::blob;

                default: return ValueType::null;
            }
        }


        bool isNull() const {
            return std::holds_alternative<std::monostate>(v_);
        }

        std::optional<std::int64_t> asInt64() const {
            if (const std::int64_t* i = std::get_if<std::int64_t>(&v_))
                return *i;

            return std::nullopt;
        }

        std::optional<double> asDouble() const {
            if (const double* p = std::get_if<double>(&v_))
                return *p;

            return std::nullopt;
        }

        std::optional<bool> asBool() const {
            if (const bool* b = std::get_if<bool>(&v_))
                return *b;

            return std::nullopt;
        }

        std::optional<Text> asString() const {
            if (const Text* t = std::get_if<Text>(&v_))
                return *t;

            return std::nullopt;
        }

        std::optional<Blob> asBlob() const {
            if (const Blob* b = std::get_if<Blob>(&v_))
                return *b;

            return std::nullopt;
        }

        std::string toString() const {
            return static_cast<std::string>();
        }

        bool operator==(const Value& other) const {
            return v_ == other.v_;
        }

        private:
            Var v_;
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

    inline Value makeBlob(Blob v) {
        return Value {
            Value::Var{std::in_place_type<Blob>, std::move(v)}
        };
    }
}