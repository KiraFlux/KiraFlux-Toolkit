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
#include "kf/mixin/Invariant.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf {

struct OptionTag {};

template<typename> struct Option;

namespace internal {

/// @brief Helper for constructing Option with a value
/// @details Provides a static method `create` that bypasses the private constructor.
struct SomeCreator final {

    template<typename T> [[nodiscard]] static constexpr Option<std::decay_t<T>> create(T &&value) noexcept {
        return {std::forward<T>(value)};
    }

    template<typename T> [[nodiscard]] static constexpr Option<T &> createRef(T &ref) noexcept {
        return {ref};
    }

    template<typename T> [[nodiscard]] static constexpr Option<Slice<T>> create(Slice<T> slice) noexcept {
        return {slice};
    }
};

}// namespace internal

template<> struct Option<void> final :

    OptionTag,
    mixin::Invariant<Option<void>>,
    mixin::Resettable<Option<void>>

{
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept : _is_some{false} {}

    /// @brief Default contructor
    constexpr Option() noexcept : _is_some{false} {}

    /// @note called by some()
    explicit constexpr Option(internal::SomeCreator) noexcept : _is_some{true} {}

private:
    bool _is_some;

    using This = Option<void>;

    KF_IMPL_INVARIANT(This);
    constexpr bool isSomeImpl() const noexcept { return _is_some; }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        if (this->isSome()) {
            _is_some = false;
        }
    }
};

/// @brief Create an Option containing a value or function (Some)
/// @tparam T Type of the value (auto‑deduced)
/// @param value The value to store
/// @return Option<T> containing the value.
template<typename T> [[nodiscard]] constexpr Option<std::decay_t<T>> some(T &&value) noexcept {
    return internal::SomeCreator::create(std::forward<T>(value));
}

[[nodiscard]] constexpr Option<void> some() noexcept {
    return Option<void>{internal::SomeCreator{}};
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

/// @brief Optional value container for value types
/// @tparam T Value type (must be trivially copyable and destructible)
/// @note Embedded‑friendly implementation without exceptions or heap allocation
///       Use kf::some() to create an Option with a value, and kf::none for an empty one
/// @see kf::some, kf::none
template<typename T> struct Option final :

    OptionTag,
    mixin::Invariant<Option<T>>,
    mixin::Resettable<Option<T>>

{
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept : _is_some{false} {}

    /// @brief Default contructor
    constexpr Option() noexcept : _is_some{false} {}

    Option(const Option &other) noexcept : _is_some{other._is_some} {
        if (this->isSome()) {
            construct(other.value());
        }
    }

    Option(Option &&other) noexcept : _is_some{other._is_some} {
        if (other.isSome()) {
            construct(std::move(other.value()));
            other.reset();
        }
    };

    Option &operator=(const Option &other) noexcept {
        if (this != &other) {
            if (other.isSome()) {
                if (this->isSome()) {
                    this->value() = other.value();
                } else {
                    construct(other.value());
                }
            } else {
                this->reset();
            }
        }

        return *this;
    }

    Option &operator=(Option &&other) noexcept {
        if (this != &other) {
            this->reset();

            if (other.isSome()) {
                construct(std::move(other.value()));
                other.reset();
            }
        }

        return *this;
    }

    ~Option() noexcept { this->reset(); }

    /// @brief Get mutable access the stored value
    /// @note (abort if is none)
    [[nodiscard]] T &unwrap() & noexcept {
        if (this->isSome()) { return value(); }
        abort();
    }

    /// @brief Get readonly access to the stored value
    /// @note (abort if is none)
    [[nodiscard]] const T &unwrap() const & noexcept { return const_cast<Option *>(this)->unwrap(); }

    [[nodiscard]] T &&unwrap() && noexcept {
        if (this->isSome()) { return std::move(value()); }
        abort();
    }

    /// @brief Get the stored value or a default (for lvalue Option)
    template<typename U> [[nodiscard]] T unwrapOr(U &&default_value) const & noexcept {
        return this->isSome() ? value() : std::forward<U>(default_value);
    }

    /// @brief Get the stored value or a default (for rvalue Option – moves stored value)
    template<typename U> [[nodiscard]] T unwrapOr(U &&default_value) && noexcept {
        return this->isSome() ? std::move(value()) : std::forward<U>(default_value);
    }

    /// @brief Transform the stored value using a function
    /// @tparam F Callable type (auto‑deduced)
    /// @param f Mapping function: T -> U
    /// @return Option<U> containing the mapped value if some,
    ///         otherwise an empty Option
    template<typename F> [[nodiscard]] auto map(F &&f) const & noexcept -> Option<decltype(f(std::declval<T>()))> {
        if (this->isNone()) { return none; }

        if constexpr (std::is_void_v<decltype(f(std::declval<T>()))>) {
            f(value());
            return some();
        } else {
            return some(f(value()));
        }
    }

    template<typename F> [[nodiscard]] auto map(F &&f) && noexcept -> Option<decltype(f(std::declval<T>()))> {
        if (this->isNone()) { return none; }

        if constexpr (std::is_void_v<decltype(f(std::declval<T>()))>) {
            f(std::move(value()));
            return some();
        } else {
            return some(f(std::move(value())));
        }
    }

private:
    alignas(T) u8 _storage[sizeof(T)]{};///< Storage for value when engaged
    bool _is_some{};                    ///< Flag indicating whether value is present

    /// @brief Private constructor for a value (called by some())
    template<typename U, typename = std::enable_if_t<not std::is_same_v<std::decay_t<U>, Option>>>
    constexpr Option(U &&value) noexcept { construct(std::forward<U>(value)); }

    template<typename U> void construct(U &&value) noexcept {
        new (static_cast<void *>(_storage)) T(std::forward<U>(value));
        _is_some = true;
    }

    [[nodiscard]] T &value() noexcept { return *reinterpret_cast<T *>(_storage); }

    [[nodiscard]] const T &value() const noexcept { return const_cast<Option *>(this)->value(); }

    using This = Option<T>;

    KF_IMPL_INVARIANT(This);
    constexpr bool isSomeImpl() const noexcept { return _is_some; }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        if (this->isSome()) {
            value().~T();
            _is_some = false;
        }
    }
};

/// @brief Optional value container for lvalue references
/// @tparam T Referenced type
/// @note Stores a pointer internally, cannot hold rvalue references.
template<typename T> struct Option<T &> final :

    OptionTag,
    mixin::Invariant<Option<T &>>,
    mixin::Resettable<Option<T &>>

{
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept : _ptr{nullptr} {}

    /// @brief Default contructor
    constexpr Option() noexcept : _ptr{nullptr} {}

    /// @brief Deleted constructor for rvalue references - cannot store temporaries
    Option(T &&) = delete;

    /// @brief Get the stored reference (undefined behaviour if not some)
    [[nodiscard]] T &unwrap() noexcept {
        if (this->isSome()) { return *_ptr; }
        abort();
    }

    /// @brief Const overload of value()
    [[nodiscard]] const T &unwrap() const noexcept {
        return const_cast<Option *>(this)->unwrap();
    }

    /// @brief Get the stored reference or a default
    /// @param default_ref Reference to return if Option is empty
    /// @return Reference to the stored object or default_ref
    [[nodiscard]] constexpr T &unwrapOr(T &default_ref) const noexcept {
        return this->isSome() ? *_ptr : default_ref;
    }

private:
    T *_ptr;

    /// @note called by someRef()
    constexpr Option(T &ref) noexcept : _ptr{&ref} {}

    using This = Option<T &>;

    KF_IMPL_INVARIANT(This);
    constexpr bool isSomeImpl() const noexcept { return _ptr != nullptr; }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept { _ptr = nullptr; }
};

/// @brief Optional value container for slice
/// @tparam T Slice item type
template<typename T> struct Option<Slice<T>> final :

    OptionTag,
    mixin::Invariant<Option<Slice<T>>>,
    mixin::Resettable<Option<Slice<T>>>

{
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept : _ptr{nullptr}, _size{is_none_mark} {}

    /// @brief Default contructor
    constexpr Option() noexcept : _ptr{nullptr}, _size{is_none_mark} {}

    /// @brief Get the stored slice
    [[nodiscard]] Slice<T> unwrap() const noexcept {
        if (this->isSome()) { return {_ptr, _size}; }
        abort();
    }

    /// @brief Get the stored slice or a default
    /// @param default_slice Slice to return if Option is empty
    /// @return The stored slice if Some, otherwise `default_slice`
    [[nodiscard]] constexpr Slice<T> unwrapOr(Slice<T> default_slice) const noexcept {
        return this->isSome() ? Slice<T>{_ptr, _size} : default_slice;
    }

private:
    static constexpr auto is_none_mark{static_cast<usize>(-1)};

    T *_ptr;
    usize _size;

    /// @note called by some()
    constexpr Option(Slice<T> slice) noexcept : _ptr{slice.data()}, _size{slice.size()} {}

    using This = Option<Slice<T>>;

    KF_IMPL_INVARIANT(This);
    constexpr bool isSomeImpl() const noexcept { return _size != is_none_mark; }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _size = is_none_mark;
        _ptr = nullptr;
    }
};

/// @brief Optional value container for function
/// @tparam R Function's return value type
/// @tparam Args Function's argument types
template<typename R, typename... Args> struct Option<Function<R(Args...)>> final :

    OptionTag,
    mixin::Invariant<Option<Function<R(Args...)>>>,
    mixin::Resettable<Option<Function<R(Args...)>>>

{
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
    ~Option() noexcept { this->reset(); }

    /// @brief Move assignment operator
    /// @param other Source Option (becomes None after move)
    Option &operator=(Option &&other) noexcept {
        if (this != &other) {
            _function = std::move(other._function);
        }
        return *this;
    }

    /// @brief Get stored function (const lvalue reference). Panics if None
    [[nodiscard]] const FunctionType &unwrap() const & noexcept {
        if (this->isNone()) { abort(); }
        return _function;
    }

    /// @brief Extract stored function (rvalue). Panics if None
    [[nodiscard]] FunctionType unwrap() && noexcept {
        if (this->isNone()) { abort(); }
        auto ret = std::move(_function);
        this->reset();
        return ret;
    }

private:
    FunctionType _function;

    /// @brief Private constructor for Some (called by kf::some)
    template<typename F> Option(F &&function) noexcept : _function{std::forward<F>(function)} {}

    using This = Option<FunctionType>;

    KF_IMPL_INVARIANT(This);
    constexpr bool isSomeImpl() const noexcept { return nullptr != _function._func; }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        if (this->isSome()) {
            _function = std::move(FunctionType{});
        }
    }
};

}// namespace kf