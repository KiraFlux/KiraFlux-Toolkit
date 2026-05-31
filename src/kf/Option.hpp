// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>

#include "kf/Function.hpp"
#include "kf/NoneType.hpp"
#include "kf/Slice.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf {

template<typename> struct Option;

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

    template<typename R, typename... Args> [[nodiscard]] static constexpr Option<Function<R(Args...)>> create(Function<R(Args...)> func) noexcept {
        return {std::move(func)};
    }
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

/// @brief Create an Option containing a function (Some)
/// @return Option<Function<R(Args...)>> containing the function object
template<typename R, typename... Args> [[nodiscard]] constexpr Option<Function<R(Args...)>> some(Function<R(Args...)> func) noexcept {
    return internal::SomeCreator::create(std::move(func));
}

/// @brief Optional value container for value types
/// @tparam T Value type (must be trivially copyable and destructible)
/// @note Embedded‑friendly implementation without exceptions or heap allocation
///       Use kf::some() to create an Option with a value, and kf::none for an empty one
/// @see kf::some, kf::none
template<typename T> struct Option final : mixin::Resettable<Option<T>> {
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept : _is_some{false} {}

    /// @brief Default contructor
    constexpr Option() noexcept : _is_some{false} {}

    Option(const Option &other) noexcept : _is_some{other._is_some} {
        if (isSome()) {
            construct(other.rawValue());
        }
    }

    Option(Option &&other) noexcept : _is_some{other._is_some} {
        if (other.isSome()) {
            construct(std::move(other.rawValue()));
            other.reset();
        }
    };

    Option &operator=(const Option &other) noexcept {
        if (this == &other) { return *this; }

        if (other.isSome()) {
            if (isSome()) {
                this->rawValue() = other.rawValue();
            } else {
                construct(other.rawValue());
            }
        } else {
            this->reset();
        }

        return *this;
    }

    Option &operator=(Option &&other) noexcept {
        if (this == &other) { return *this; }

        this->reset();

        if (other.isSome()) {
            construct(std::move(other.rawValue()));
            other.reset();
        }

        return *this;
    }

    ~Option() noexcept { this->reset(); }

    /// @brief Check if Option contains a value
    [[nodiscard]] constexpr bool isSome() const noexcept { return _is_some; }

    /// @brief Check if Option does not contain a value
    [[nodiscard]] constexpr bool isNone() const noexcept { return not isSome(); }

    /// @brief Get the stored value (undefined behaviour if not some)
    [[nodiscard]] T &value() & noexcept {
        if (isSome()) { return rawValue(); }
        abort();
    }

    [[nodiscard]] const T &value() const & noexcept { return const_cast<Option *>(this)->value(); }

    [[nodiscard]] T &&value() && noexcept {
        if (isSome()) { return std::move(rawValue()); }
        abort();
    }

    /// @brief Get the stored value or a default (for lvalue Option)
    [[nodiscard]] T valueOr(const T &default_value) const & noexcept {
        return isSome() ? rawValue() : default_value;
    }

    [[nodiscard]] T valueOr(T &&default_value) const & noexcept {
        return isSome() ? rawValue() : std::move(default_value);
    }

    /// @brief Get the stored value or a default (for rvalue Option – moves stored value)
    [[nodiscard]] T &&valueOr(const T &default_value) && noexcept {
        return isSome() ? std::move(rawValue()) : default_value;
    }

    [[nodiscard]] T &&valueOr(T &&default_value) && noexcept {
        return isSome() ? std::move(rawValue()) : std::move(default_value);
    }

    /// @brief Transform the stored value using a function
    /// @tparam F Callable type (auto‑deduced)
    /// @param f Mapping function: T -> U
    /// @return Option<U> containing the mapped value if some,
    ///         otherwise an empty Option
    template<typename F> [[nodiscard]] auto map(F &&f) const noexcept -> Option<decltype(f(std::declval<T>()))> {
        return isSome() ? some(f(rawValue())) : none;
    }

private:
    alignas(T) u8 _storage[sizeof(T)]{};///< Storage for value when engaged
    bool _is_some{};                    ///< Flag indicating whether value is present

    /// @brief Private constructor for a value (called by some())
    constexpr Option(T value) noexcept { construct(std::move(value)); }

    template<typename U> void construct(U &&value) noexcept {
        new (static_cast<void *>(_storage)) T(std::forward<U>(value));
        _is_some = true;
    }

    [[nodiscard]] T &rawValue() noexcept { return *reinterpret_cast<T *>(_storage); }

    [[nodiscard]] const T &rawValue() const noexcept { return const_cast<Option *>(this)->rawValue(); }

    KF_IMPL_RESETTABLE(Option<T>);
    void resetImpl() noexcept {
        if (isSome()) {
            rawValue().~T();
            _is_some = false;
        }
    }
};

/// @brief Optional value container for lvalue references
/// @tparam T Referenced type
/// @note Stores a pointer internally, cannot hold rvalue references.
template<typename T> struct Option<T &> final : mixin::Resettable<Option<T &>> {
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept : _ptr{nullptr} {}

    /// @brief Default contructor
    constexpr Option() noexcept : _ptr{nullptr} {}

    /// @brief Deleted constructor for rvalue references - cannot store temporaries
    Option(T &&) = delete;

    /// @brief Check if Option contains a reference
    [[nodiscard]] constexpr bool isSome() const noexcept { return _ptr != nullptr; }

    /// @brief Check if Option does not contain a reference
    [[nodiscard]] constexpr bool isNone() const noexcept { return nullptr == _ptr; }

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
    constexpr Option(NoneType) noexcept : _ptr{nullptr}, _size{is_none_mark} {}

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
    void resetImpl() noexcept {
        _size = is_none_mark;
        _ptr = nullptr;
    }
};

/// @brief Optional value container for function
/// @tparam R Function's return value type
/// @tparam Args Function's argument types
template<typename R, typename... Args> struct Option<Function<R(Args...)>> final : mixin::Resettable<Option<Function<R(Args...)>>> {
    friend struct internal::SomeCreator;

    using FunctionType = Function<R(Args...)>;

    /// @brief Default constructor - creates empty Option (None)
    constexpr Option() noexcept : _function{} {}

    /// @brief Construct empty Option from NoneType (used with kf::none)
    constexpr Option(NoneType) noexcept : _function{} {}

    /// @brief Move constructor
    /// @param other Source Option (becomes None after move)
    Option(Option &&other) noexcept : _function{std::move(other._function)} {}

    /// @brief Destructor - destroys stored function if present
    ~Option() { this->reset(); }

    /// @brief Move assignment operator
    /// @param other Source Option (becomes None after move)
    Option &operator=(Option &&other) noexcept {
        if (this != &other) {
            _function = std::move(other._function);
        }
        return *this;
    }

    /// @brief Check if Option contains a function
    [[nodiscard]] constexpr bool isSome() const noexcept { return nullptr != _function._func; }

    /// @brief Check if Option is empty
    [[nodiscard]] constexpr bool isNone() const noexcept { return nullptr == _function._func; }

    /// @brief Get stored function (const lvalue reference). Panics if None
    [[nodiscard]] const FunctionType &value() const & {
        if (isNone()) { abort(); }

        return _function;
    }

    /// @brief Extract stored function (rvalue). Panics if None
    [[nodiscard]] FunctionType value() && {
        if (isNone()) { abort(); }

        auto ret = std::move(_function);
        this->reset();
        return ret;
    }

private:
    FunctionType _function;

    /// @brief Private constructor for Some (called by kf::some)
    Option(FunctionType function) noexcept : _function{std::move(function)} {}

    KF_IMPL_RESETTABLE(Option<FunctionType>);
    void resetImpl() noexcept {
        if (isSome()) {
            _function = std::move(FunctionType{});
        }
    }
};

}// namespace kf