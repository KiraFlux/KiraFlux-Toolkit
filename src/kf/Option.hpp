// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "kf/Slice.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf {

template<typename T> struct Option;

namespace internal {

/// @brief Helper for constructing Option with a value
/// @details Provides a static method `create` that bypasses the private constructor.
struct SomeCreator final {
    template<typename T> [[nodiscard]] static constexpr Option<T> create(T value) noexcept {
        return {std::move(value)};
    }

    template<typename T> [[nodiscard]] static constexpr Option<T &> createRef(T &ref) noexcept {
        return {ref};
    }

    template<typename T> [[nodiscard]] static constexpr Option<Slice<T>> create(Slice<T> slice) noexcept {
        return {slice};
    }
};

/// @brief Tag type for constructing an empty Option (None)
/// @note Use the global constant kf::none to create empty options.
struct NoneType final {
    explicit NoneType() = default;
};

}// namespace internal

/// @brief Create an Option containing a value (Some)
/// @tparam T Type of the value (auto‑deduced)
/// @param value The value to store
/// @return Option<T> containing the value.
template<typename T> [[nodiscard]] constexpr Option<T> some(T value) noexcept {
    return internal::SomeCreator::create(std::move(value));
}

/// @brief Create an Option containing a reference (Some_ref)
/// @tparam T Type of the referenced value
/// @param ref Reference to store
/// @return Option<T&> containing the reference.
template<typename T> [[nodiscard]] constexpr Option<T &> someRef(T &ref) noexcept {
    return internal::SomeCreator::createRef(ref);
}

/// @brief Create an Option containing a slice (Some)
/// @tparam T Type of an slice item (auto‑deduced)
/// @return Option<Slice<T>> containing the slice.
template<typename T> [[nodiscard]] constexpr Option<Slice<T>> some(Slice<T> slice) noexcept {
    return internal::SomeCreator::create(slice);
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
    constexpr Option(internal::NoneType) noexcept : _is_some{false}, _dummy{} {}

    /// @brief Default contructor
    constexpr Option() noexcept : _is_some{false}, _dummy{} {}

    /// @brief Check if Option contains a value
    [[nodiscard]] constexpr bool isSome() const noexcept { return _is_some; }

    /// @brief Check if Option does not contain a value
    [[nodiscard]] constexpr bool isNone() const noexcept { return not _is_some; }

    /// @brief Get the stored value (undefined behaviour if not some)
    [[nodiscard]] T &value() noexcept {
        if (isSome()) { return _value; }
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
        return isSome() ? _value : std::move(default_value);
    }

    /// @brief Transform the stored value using a function
    /// @tparam F Callable type (auto‑deduced)
    /// @param f Mapping function: T -> U
    /// @return Option<U> containing the mapped value if some,
    ///         otherwise an empty Option
    template<typename F> [[nodiscard]] auto map(F &&f) const noexcept -> Option<decltype(f(std::declval<T>()))> {
        return isSome() ? some(f(_value)) : none;
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
    constexpr Option(internal::NoneType) noexcept : _ptr{nullptr} {}

    /// @brief Default contructor
    constexpr Option() noexcept : _ptr{nullptr} {}

    /// @brief Deleted constructor for rvalue references – cannot store temporaries
    Option(T &&) = delete;

    /// @brief Check if Option contains a reference
    [[nodiscard]] constexpr bool isSome() const noexcept { return _ptr != nullptr; }

    /// @brief Check if Option does not contain a reference
    [[nodiscard]] constexpr bool isNone() const noexcept { return _ptr == nullptr; }

    /// @brief Get the stored reference (undefined behaviour if not some)
    [[nodiscard]] T &value() noexcept {
        if (isSome()) { return *_ptr; }
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
        return isSome() ? *_ptr : default_ref;
    }

private:
    T *_ptr;///< Pointer to the referenced object (nullptr indicates None)

    /// @note called by someRef()
    constexpr Option(T &ref) noexcept : _ptr{&ref} {}

    KF_IMPL_RESETTABLE(Option<T &>);
    void resetImpl() noexcept { _ptr = nullptr; }
};

/// @brief Optional value container for slice
/// @tparam T Slice item type
template<typename T> struct Option<Slice<T>> final : mixin::Resettable<Option<Slice<T>>> {
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(internal::NoneType) noexcept : _ptr{nullptr}, _size{is_none_mark} {}

    /// @brief Default contructor
    constexpr Option() noexcept : _ptr{nullptr}, _size{is_none_mark} {}

    /// @brief Check if Option contains a reference
    [[nodiscard]] constexpr bool isSome() const noexcept { return _size != is_none_mark; }

    /// @brief Check if Option does not contain a reference
    [[nodiscard]] constexpr bool isNone() const noexcept { return _size == is_none_mark; }

    /// @brief Get the stored slice
    [[nodiscard]] Slice<T> value() const noexcept {
        if (isSome()) { return {_ptr, _size}; }
        abort();
    }

    /// @brief Get the stored slice or a default
    /// @param default_slice Slice to return if Option is empty
    /// @return The stored slice if Some, otherwise `default_slice`
    [[nodiscard]] constexpr Slice<T> valueOr(Slice<T> default_slice) const noexcept {
        return isSome() ? Slice<T>{_ptr, _size} : default_slice;
    }

private:
    static constexpr auto is_none_mark{static_cast<usize>(-1)};

    T *_ptr;
    usize _size;

    /// @note called by some()
    constexpr Option(Slice<T> slice) noexcept : _ptr{slice.data()}, _size{slice.size()} {}

    KF_IMPL_RESETTABLE(Option<Slice<T>>);
    void resetImpl() noexcept { _size = is_none_mark; }
};

}// namespace kf