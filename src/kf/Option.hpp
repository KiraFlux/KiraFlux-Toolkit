// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "kf/mixin/Resettable.hpp"

namespace kf {

template<typename T> struct Option;

namespace internal {

/// @brief Helper for constructing Option with a value.
/// @details Provides a static method `create` that bypasses the private constructor.
struct SomeCreator final {
    template<typename U> [[nodiscard]] static constexpr Option<std::decay_t<U>> create(U &&value) noexcept {
        return {std::forward<U>(value)};
    }
};

/// @brief Tag type for constructing an empty Option (None).
/// @note Use the global constant kf::none to create empty options.
struct NoneType final {
    explicit NoneType() = default;
};

}// namespace internal

/// @brief Create an Option containing a value (Some)
/// @tparam U Type of the value (auto‑deduced).
/// @param value The value to store.
/// @return Option<U> containing the value.
template<typename U> [[nodiscard]] constexpr Option<std::decay_t<U>> some(U &&value) noexcept {
    return internal::SomeCreator::create(std::forward<U>(value));
}

/// @brief Global constant for constructing an empty Option (None)
constexpr internal::NoneType none{};

/// @brief Optional value container (similar to std::optional)
/// @tparam T Value type (must be trivially copyable and destructible)
/// @note Embedded‑friendly implementation without exceptions or heap allocation.
///       Use kf::some() to create an Option with a value, and kf::none for an empty one.
/// @see kf::some, kf::none
template<typename T> struct Option final : mixin::Resettable<Option<T>> {
    static_assert(std::is_trivially_destructible_v<T>);
    static_assert(std::is_trivially_copyable_v<T>);

    friend struct ::kf::internal::SomeCreator;

    /// @brief Check if Option contains a value.
    [[nodiscard]] constexpr bool isSome() const noexcept { return _is_some; }

    /// @brief Check if Option does not contain a value.
    [[nodiscard]] constexpr bool isNone() const noexcept { return not _is_some; }

    /// @brief Get the stored value (panic if not some)
    [[nodiscard]] T &value() noexcept {
        if (_is_some) { return _value; }
        abort();
    }

    /// @brief Get Readonly access the stored value (panic if not some)
    [[nodiscard]] const T &value() const noexcept {
        return const_cast<Option *>(this)->value();
    }

    /// @brief Get the stored value or a default.
    /// @param default_value Value to return if Option is empty.
    /// @note Safe alternative to value().
    [[nodiscard]] constexpr T valueOr(T default_value) const noexcept {
        return _is_some ? _value : std::move(default_value);
    }

    /// @brief Transform the stored value using a function.
    /// @tparam F Callable type (auto‑deduced).
    /// @param f Mapping function: T -> U.
    /// @return Option<U> containing the mapped value if some,
    ///         otherwise an empty Option.
    template<typename F> [[nodiscard]] auto map(F &&f) const noexcept -> Option<decltype(f(std::declval<T>()))> {
        if (isSome()) {
            return some(f(_value));
        } else {
            return {none};
        }
    }

    /// @brief Construct an empty Option (None)
    constexpr Option(internal::NoneType) noexcept : _is_some{false}, _dummy{0} {}

private:
    union {
        T _value;   ///< Storage for value when engaged
        char _dummy;///< Dummy member for empty state
    };
    bool _is_some;///< Flag indicating whether value is present

    /// @brief Private constructor for a value (called by some()).
    constexpr Option(T value) noexcept : _is_some{true}, _value{std::move(value)} {}

    KF_IMPL_RESETTABLE(::kf::Option<T>);
    void resetImpl() noexcept { _is_some = false; }
};

}// namespace kf