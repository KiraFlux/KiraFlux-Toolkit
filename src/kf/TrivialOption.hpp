// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>
#include <cstdlib>
#include <limits>
#include <type_traits>

#include "kf/NoneType.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/Invariant.hpp"
#include "kf/mixin/Resettable.hpp"
#include "kf/primitives.hpp"

namespace kf {

/// @brief Tag struct for identifying Option types
struct OptionTag {};

template<typename T> struct TrivialOption;

/// @brief Trivially copyable optional container for trivially copyable types
/// @tparam T Stored type
/// @note Safe for byte serialization
/// @note Created via `kf::someTrivial()` or `kf::none`
/// @note Always check `isSome()` before `unwrap()`, otherwise `abort()`
template<trivial T>
    requires(not enum_type<T> and not float_type<T>)
struct TrivialOption<T> :

    OptionTag,
    mixin::Invariant<TrivialOption<T>>,
    mixin::Resettable<TrivialOption<T>>

{
    using Self = TrivialOption<T>;

    /// @brief Construct an empty Option (None)
    constexpr TrivialOption(NoneType) noexcept :
        _dummy{}, _is_some{false} {}

    /// @brief Default constructor – empty Option (None)
    /// @note Intended for containers
    constexpr TrivialOption() noexcept :
        _dummy{}, _is_some{false} {}

    /// @brief Create Option with value (Some)
    constexpr TrivialOption(T const &value) noexcept :
        _value{value}, _is_some{true} {}

    /// @brief Unwrap the stored value (lvalue Option)
    /// @return Mutable reference to stored value
    /// @note Aborts if None
    [[nodiscard]] T &unwrap() & noexcept {
        if (this->isNone()) { abort(); }
        return _value;
    }

    /// @brief Unwrap the stored value (const lvalue Option)
    /// @return Const reference to stored value
    /// @note Aborts if None
    [[nodiscard]] T const &unwrap() const & noexcept {
        return const_cast<TrivialOption *>(this)->unwrap();
    }

    /// @brief Unwrap the stored value (rvalue Option)
    /// @return Rvalue reference to stored value (allows moving out)
    /// @note Aborts if None
    [[nodiscard]] T &&unwrap() && noexcept {
        if (this->isNone()) { abort(); }
        return std::move(_value);
    }

    /// @brief Get stored value or a default
    /// @param default_value Value to return if Option is empty
    /// @return Stored value if Some, otherwise default_value (copied)
    [[nodiscard]] constexpr T unwrapOr(T default_value) const noexcept {
        return this->isSome() ? _value : default_value;
    }

private:
    union {
        T _value;
        char _dummy;
    };
    bool _is_some;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _is_some;
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        _is_some = false;
    }
};

template<enum_type E> struct TrivialOption<E> :

    OptionTag,
    mixin::Invariant<TrivialOption<E>>,
    mixin::Resettable<TrivialOption<E>>

{
    using Self = TrivialOption<E>;

    using Underlying = std::underlying_type_t<E>;

    constexpr TrivialOption(NoneType) noexcept :
        _value{none_value} {}

    constexpr TrivialOption() noexcept :
        _value{none_value} {}

    constexpr TrivialOption(E value) noexcept :
        _value{static_cast<Underlying>(value)} {}

    [[nodiscard]] E unwrap() const noexcept {
        if (this->isNone()) { abort(); }
        return static_cast<E>(_value);
    }

    [[nodiscard]] E unwrapOr(E default_value) const noexcept {
        return this->isSome() ? static_cast<E>(_value) : default_value;
    }

private:
    static constexpr auto none_value{static_cast<Underlying>(-1)};

    Underlying _value;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _value != none_value;
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        _value = none_value;
    }
};

template<> struct TrivialOption<usize> :

    OptionTag,
    mixin::Invariant<TrivialOption<usize>>,
    mixin::Resettable<TrivialOption<usize>>

{

    using Self = TrivialOption<usize>;

    /// @brief Construct an empty Option (None)
    constexpr TrivialOption(NoneType) noexcept :
        _value{is_none_mark} {}

    /// @brief Default constructor – empty Option (None)
    /// @note Intended for containers
    constexpr TrivialOption() noexcept :
        _value{is_none_mark} {}

    constexpr TrivialOption(usize value) noexcept :
        _value{value} {}

    /// @brief Unwrap the stored value
    /// @return Mutable reference to stored value
    /// @note Aborts if None
    [[nodiscard]] usize &unwrap() noexcept {
        if (this->isNone()) { abort(); }
        return _value;
    }

    /// @brief Unwrap the stored value
    /// @return stored size
    /// @note Aborts if None
    [[nodiscard]] usize unwrap() const noexcept {
        return const_cast<TrivialOption *>(this)->unwrap();
    }

    /// @brief Get stored value or a default
    /// @param default_value Value to return if Option is empty
    /// @return Stored value if Some, otherwise default_value (copied)
    [[nodiscard]] constexpr usize unwrapOr(usize default_value) const noexcept {
        return this->isSome() ? _value : default_value;
    }

private:
    static constexpr auto is_none_mark{static_cast<usize>(-1)};

    usize _value;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _value != is_none_mark;
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        _value = is_none_mark;
    }
};

template<float_type T> struct TrivialOption<T> :

    OptionTag,
    mixin::Invariant<TrivialOption<T>>,
    mixin::Resettable<TrivialOption<T>>

{
    using Self = TrivialOption<T>;

    constexpr TrivialOption(NoneType) noexcept :
        _value{nan} {}

    constexpr TrivialOption() noexcept :
        _value{nan} {}

    constexpr TrivialOption(T value) noexcept :
        _value{value} {}

    [[nodiscard]] T &unwrap() noexcept {
        if (this->isNone()) { abort(); }
        return _value;
    }

    [[nodiscard]] T const &unwrap() const noexcept {
        return const_cast<TrivialOption *>(this)->unwrap();
    }

    [[nodiscard]] T unwrapOr(T default_value) const noexcept {
        return this->isSome() ? _value : default_value;
    }

private:
    static constexpr auto nan{std::numeric_limits<T>::quiet_NaN()};

    T _value;

    KF_IMPL_INVARIANT(Self);
    bool isSomeImpl() const noexcept {
        return not std::isnan(_value);
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        _value = nan;
    }
};

/// @brief Create TrivialOption containing a value
/// @param value The value to store (copied)
/// @return `TrivialOption<T>`
[[nodiscard]] constexpr auto someTrivial(trivial auto const &value) noexcept {
    return TrivialOption<std::remove_cvref_t<decltype(value)>>(value);
}

}// namespace kf