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

/// @brief Helper for constructing Option with a value
/// @details Provides a static method `create` that bypasses the private constructor.
struct SomeCreator final {
    template<typename U> [[nodiscard]] static constexpr Option<std::decay_t<U>> create(U &&value) noexcept {
        return {std::forward<U>(value)};
    }

    template<typename U> [[nodiscard]] static constexpr Option<U &> createRef(U &ref) noexcept {
        return {ref};
    }
};

/// @brief Tag type for constructing an empty Option (None)
/// @note Use the global constant kf::none to create empty options.
struct NoneType final {
    explicit NoneType() = default;
};

}// namespace internal

/// @brief Create an Option containing a value (Some)
/// @tparam U Type of the value (auto‑deduced)
/// @param value The value to store
/// @return Option<U> containing the value.
template<typename U> [[nodiscard]] constexpr Option<std::decay_t<U>> some(U &&value) noexcept {
    return internal::SomeCreator::create(std::forward<U>(value));
}

/// @brief Create an Option containing a reference (Some_ref)
/// @tparam U Type of the referenced value
/// @param ref Reference to store
/// @return Option<U&> containing the reference.
template<typename U> [[nodiscard]] constexpr Option<U &> someRef(U &ref) noexcept {
    return internal::SomeCreator::createRef(ref);
}

/// @brief Global constant for constructing an empty Option (None).
constexpr internal::NoneType none{};

/// @brief Optional value container for value types
/// @tparam T Value type (must be trivially copyable and destructible)
/// @note Embedded‑friendly implementation without exceptions or heap allocation
///       Use kf::some() to create an Option with a value, and kf::none for an empty one
/// @see kf::some, kf::none
template<typename T> struct Option final : mixin::Resettable<Option<T>> {
    static_assert(std::is_trivially_destructible_v<T>);
    static_assert(std::is_trivially_copyable_v<T>);

    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(internal::NoneType) noexcept : _is_some{false}, _dummy{0} {}

    /// @brief Check if Option contains a value
    [[nodiscard]] constexpr bool isSome() const noexcept { return _is_some; }

    /// @brief Check if Option does not contain a value
    [[nodiscard]] constexpr bool isNone() const noexcept { return not _is_some; }

    /// @brief Get the stored value (undefined behaviour if not some)
    [[nodiscard]] T &value() noexcept {
        if (_is_some) return _value;
        abort();
    }

    /// @brief Const overload of value()
    [[nodiscard]] const T &value() const noexcept {
        return const_cast<Option *>(this)->value();
    }

    /// @brief Get the stored value or a default
    /// @param default_value Value to return if Option is empty
    /// @note Safe alternative to value()
    [[nodiscard]] constexpr T valueOr(T default_value) const noexcept {
        return _is_some ? _value : std::move(default_value);
    }

    /// @brief Transform the stored value using a function
    /// @tparam F Callable type (auto‑deduced)
    /// @param f Mapping function: T -> U
    /// @return Option<U> containing the mapped value if some,
    ///         otherwise an empty Option
    template<typename F> [[nodiscard]] auto map(F &&f) const noexcept -> Option<decltype(f(std::declval<T>()))> {
        if (isSome()) {
            return some(f(_value));
        } else {
            return {none};
        }
    }

private:
    union {
        T _value;   ///< Storage for value when engaged
        char _dummy;///< Dummy member for empty state
    };
    bool _is_some;///< Flag indicating whether value is present

    /// @brief Private constructor for a value (called by some())
    constexpr Option(T value) noexcept : _is_some{true}, _value{std::move(value)} {}

    KF_IMPL_RESETTABLE(Option<T>);
    void resetImpl() noexcept { _is_some = false; }
};

/// @brief Optional value container for lvalue references
/// @tparam T Referenced type
/// @note Stores a pointer internally, cannot hold rvalue references.
template<typename T> struct Option<T &> final : mixin::Resettable<Option<T &>> {
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(internal::NoneType) noexcept : _ptr(nullptr) {}

    /// @brief Construct an Option storing a reference to `ref`
    constexpr Option(T &ref) noexcept : _ptr(&ref) {}

    /// @brief Deleted constructor for rvalue references – cannot store temporaries
    Option(T &&) = delete;

    /// @brief Check if Option contains a reference
    [[nodiscard]] constexpr bool isSome() const noexcept { return _ptr != nullptr; }

    /// @brief Check if Option does not contain a reference
    [[nodiscard]] constexpr bool isNone() const noexcept { return _ptr == nullptr; }

    /// @brief Get the stored reference (undefined behaviour if not some)
    [[nodiscard]] T &value() noexcept {
        if (_ptr) return *_ptr;
        abort();
    }

    /// @brief Const overload of value()
    [[nodiscard]] const T &value() const noexcept {
        return const_cast<Option *>(this)->value();
    }

    /// @brief Get the stored reference or a default
    /// @param default_ref Reference to return if Option is empty
    /// @return Reference to the stored object or default_ref
    [[nodiscard]] constexpr T &valueOr(T &default_ref) const noexcept {
        return _ptr ? *_ptr : default_ref;
    }

    /// @brief Reset the Option to empty state (None)
    void reset() noexcept { _ptr = nullptr; }

private:
    T *_ptr;///< Pointer to the referenced object (nullptr indicates None)

    KF_IMPL_RESETTABLE(Option<T &>);
    void resetImpl() noexcept { _ptr = nullptr; }
};

}// namespace kf