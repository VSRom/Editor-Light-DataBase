#pragma once

#include <variant>
#include <optional>
#include <cassert>
#include <type_traits>
#include <utility>

[[noreturn]] inline void unreachable() noexcept {
#if defined(__clang__) || defined(__GNUC__)
    __builtin_unreachable();
#elif defined(_MSC_VER)
    __assume(0);
#else
#error "Unknown compiler: unreachable() is not implemented."
#endif
}

template <typename T, typename E>
class [[nodiscard]] Result {
public:
    static_assert(!std::is_same_v<T, E>, "Result<T,E> requires distinct T and E");
    static_assert(!std::is_void_v<T> && !std::is_void_v<E>, "Use void specialization for Result<void, E>.");

    explicit Result(T value) noexcept
        : v_(std::move(value))
    { }

    explicit Result(E error) noexcept
        : v_(std::move(error))
    { }

    Result(const Result&) = default;
    Result(Result&&) noexcept = default;
    Result& operator=(const Result&) = default;
    Result& operator=(Result&&) noexcept = default;
    ~Result() = default;

    static Result Ok(T value)  { return Result(std::move(value)); }
    static Result Err(E error) { return Result(std::move(error)); }

    T& operator*() & {
        assert(has_value());
        if (!has_value()) [[unlikely]] unreachable();
        return std::get<T>(v_);
    }

    T* operator->() & {
        assert(has_value());
        if (!has_value()) [[unlikely]] unreachable();
        return &std::get<T>(v_);
    }

    const T& operator*() const & {
        assert(has_value());
        if (!has_value()) [[unlikely]] unreachable();
        return std::get<T>(v_);
    }

    const T* operator->() const & {
        assert(has_value());
        if (!has_value()) [[unlikely]] unreachable();
        return &std::get<T>(v_);
    }

    T&& operator*() && {
        assert(has_value());
        if (!has_value()) [[unlikely]] unreachable();
        return std::get<T>(std::move(v_));
    }

    template <typename U>
    T value_or(U&& defaultVal) const & {
        return has_value() ? std::get<T>(v_) : static_cast<T>(std::forward<U>(defaultVal));
    }

    template <typename U>
    T value_or(U&& defaultVal) && {
        return has_value() ? std::move(std::get<T>(v_)) : static_cast<T>(std::forward<U>(defaultVal));
    }

    E& error() & {
        assert(has_error());
        if (!has_error()) [[unlikely]] unreachable();
        return std::get<E>(v_);
    }

    const E& error() const & {
        assert(has_error());
        if (!has_error()) [[unlikely]] unreachable();
        return std::get<E>(v_);
    }

    bool has_value() const noexcept { return std::holds_alternative<T>(v_); }
    bool has_error() const noexcept { return std::holds_alternative<E>(v_); }

    explicit operator bool() const noexcept { return has_value(); }

private:
    std::variant<T, E> v_;
};

template <typename E>
class [[nodiscard]] Result<void, E> {
public:
    static_assert(!std::is_void_v<E>, "Error type cannot be void.");

    Result() : opt_error_(std::nullopt) {}

    explicit Result(E error) noexcept
        : opt_error_(std::move(error))
    { }

    Result(const Result&) = default;
    Result(Result&&) noexcept = default;
    Result& operator=(const Result&) = default;
    Result& operator=(Result&&) noexcept = default;
    ~Result() = default;

    static Result Ok() { return Result(); }
    static Result Err(E error) { return Result(std::move(error)); }

    void value() const {
        assert(has_value());
        if (!has_value()) [[unlikely]] unreachable();
    }

    E& error() & {
        assert(has_error());
        if (!has_error()) [[unlikely]] unreachable();
        return *opt_error_;
    }

    const E& error() const & {
        assert(has_error());
        if (!has_error()) [[unlikely]] unreachable();
        return *opt_error_;
    }

    bool has_value() const noexcept { return !opt_error_.has_value(); }
    bool has_error() const noexcept { return opt_error_.has_value(); }

    explicit operator bool() const noexcept { return has_value(); }

private:
    std::optional<E> opt_error_;
};
