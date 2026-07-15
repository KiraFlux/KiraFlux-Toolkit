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
#include "kf/TrivialOption.hpp"
#include "kf/mixin/Invariant.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf {

template<typename> struct Option;

namespace internal {

/// @brief Helper for constructing Option with a value
/// @note Provides a static method `create` that bypasses the private constructor.
struct SomeCreator final {

    [[nodiscard]] static constexpr auto create(auto &&value) noexcept {
        return Option<std::decay_t<decltype(value)>>{std::forward<decltype(value)>(value)};
    }

    [[nodiscard]] static constexpr auto createRef(auto &ref) noexcept {
        return Option<decltype(ref) &>{ref};
    }
};

}// namespace internal

/// @brief Specialization for void - represents optional unit (Some/None)
/// @note Has no value to unwrap; only isSome()/isNone() and reset()
/// @see kf::some() (no arguments) to create Some<void>
template<> struct Option<void> final :

    OptionTag,
    mixin::Invariant<Option<void>>,
    mixin::Resettable<Option<void>>

{
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept :
        _is_some{false} {}

    /// @brief Default constructor - empty Option (None)
    /// @note Intended for containers
    constexpr Option() noexcept :
        _is_some{false} {}

    /// @brief Construct Option<void> (called by `kf::some()`)
    explicit constexpr Option(internal::SomeCreator) noexcept :
        _is_some{true} {}

private:
    bool _is_some;

    using Self = Option<void>;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _is_some;
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        if (this->isSome()) {
            _is_some = false;
        }
    }
};

/// @brief Create Option containing a value
/// @param value The value to store (copied or moved)
/// @return `Option<std::decay_t<T>>`
/// @note This is the only way to create a non‑empty Option for value types
[[nodiscard]] constexpr auto some(auto &&value) noexcept {
    return internal::SomeCreator::create(std::forward<decltype(value)>(value));
}

/// @brief Create `Option<void>`
/// @return `Option<void>`
[[nodiscard]] constexpr auto some() noexcept {
    return Option<void>{internal::SomeCreator{}};
}

/// @brief Create Option containing a reference
/// @tparam T Referenced type
/// @param ref Reference to store
/// @return `Option<T&>`
/// @note The reference must remain valid throughout Option lifetime
[[nodiscard]] constexpr auto someRef(auto &ref) noexcept {
    return internal::SomeCreator::createRef(ref);
}

/// @brief Optional container for any value type
/// @tparam T Stored type
/// @note Created via `kf::some()` or `kf::none`
/// @note Always check `isSome()` before `unwrap()`, otherwise `abort()`
template<typename T> struct Option final :

    OptionTag,
    mixin::Invariant<Option<T>>,
    mixin::Resettable<Option<T>>

{
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept :
        _is_some{false} {}

    /// @brief Default constructor - empty Option (None)
    /// @note Intended for containers
    constexpr Option() noexcept :
        _is_some{false} {}

    /// @brief Copy constructor
    /// @param other Source Option
    /// @note Copies stored value if any
    Option(const Option &other) noexcept : _is_some{other._is_some} {
        if (this->isSome()) {
            construct(other.value());
        }
    }

    /// @brief Move constructor
    /// @param other Source Option
    /// @note Moves stored value; source becomes None
    Option(Option &&other) noexcept : _is_some{other._is_some} {
        if (other.isSome()) {
            construct(std::move(other.value()));
            other.reset();
        }
    };

    /// @brief Copy assignment
    /// @param other Source Option
    /// @return Reference to this
    /// @note Copies stored value if any
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

    /// @brief Move assignment
    /// @param other Source Option
    /// @return Reference to this
    /// @note Moves stored value; source becomes None
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

    /// @brief Destructor - destroys stored value if any
    ~Option() noexcept {
        this->reset();
    }

    /// @brief Unwrap the stored value (lvalue Option)
    /// @return Mutable reference to stored value
    /// @note Aborts if None
    [[nodiscard]] T &unwrap() & noexcept {
        if (this->isSome()) { return value(); }
        abort();
    }

    /// @brief Unwrap the stored value (const lvalue Option)
    /// @return Const reference to stored value
    /// @note Aborts if None
    [[nodiscard]] const T &unwrap() const & noexcept {
        return const_cast<Option *>(this)->unwrap();
    }

    /// @brief Unwrap the stored value (rvalue Option)
    /// @return Rvalue reference to stored value (allows moving out)
    /// @note Aborts if None
    [[nodiscard]] T &&unwrap() && noexcept {
        if (this->isSome()) { return std::move(value()); }
        abort();
    }

    /// @brief Get stored value or a default (lvalue Option)
    /// @param default_value Value to return if None
    /// @return Stored value (copy) if Some, otherwise default_value (copied or moved)
    [[nodiscard]] T unwrapOr(auto &&default_value) const & noexcept {
        return this->isSome() ? value() : std::forward<decltype(default_value)>(default_value);
    }

    /// @brief Get stored value or a default (rvalue Option - moves stored value)
    /// @param default_value Value to return if None
    /// @return Stored value (moved) if Some, otherwise default_value (copied or moved)
    [[nodiscard]] T unwrapOr(auto &&default_value) && noexcept {
        return this->isSome() ? std::move(value()) : std::forward<decltype(default_value)>(default_value);
    }

    /// @brief Transform stored value using a function (lvalue Option)
    /// @param f Mapping function: `T -> U`
    /// @return `Option<U>` containing mapped value if Some, otherwise None
    /// @note If `f` returns `void`, the result is `Option<void>`
    [[nodiscard]] auto map(auto &&f) const & noexcept -> Option<decltype(f(std::declval<T>()))> {
        if (this->isNone()) { return none; }

        if constexpr (std::is_void_v<decltype(f(std::declval<T>()))>) {
            f(value());
            return some();
        } else {
            return some(f(value()));
        }
    }

    /// @brief Transform stored value using a function (rvalue Option - moves value)
    /// @param f Mapping function: `T -> U`
    /// @return `Option<U>` containing mapped value if Some, otherwise None
    /// @note If `f` returns `void`, the result is `Option<void>`
    [[nodiscard]] auto map(auto &&f) && noexcept -> Option<decltype(f(std::declval<T>()))> {
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
    explicit constexpr Option(auto &&value) noexcept
        requires(not std::is_base_of_v<OptionTag, std::decay_t<decltype(value)>>)
    {
        construct(std::forward<decltype(value)>(value));
    }

    void construct(auto &&value) noexcept {
        new (static_cast<void *>(_storage)) T(std::forward<decltype(value)>(value));
        _is_some = true;
    }

    [[nodiscard]] T &value() noexcept {
        return *reinterpret_cast<T *>(_storage);
    }

    [[nodiscard]] const T &value() const noexcept {
        return const_cast<Option *>(this)->value();
    }

    using Self = Option<T>;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _is_some;
    }

    KF_IMPL_RESETTABLE(Self);
    void resetImpl() noexcept {
        if (this->isSome()) {
            value().~T();
            _is_some = false;
        }
    }
};

/// @brief Optional reference container
/// @tparam T Referenced type (may be const)
/// @note Stores a pointer internally, cannot hold rvalue references
/// @note Created only via `kf::someRef()`
template<typename T> struct Option<T &> final :

    OptionTag,
    mixin::Invariant<Option<T &>>,
    mixin::Resettable<Option<T &>>

{
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept :
        _ptr{nullptr} {}

    /// @brief Default constructor - empty Option (None)
    /// @note Intended for containers
    constexpr Option() noexcept :
        _ptr{nullptr} {}

    /// @brief Deleted constructor for rvalue references - cannot store temporaries
    Option(T &&) = delete;

    /// @brief Unwrap the stored reference (lvalue Option)
    /// @return Reference to stored object
    /// @note Aborts if None
    [[nodiscard]] T &unwrap() noexcept {
        if (this->isNone()) { abort(); }
        return *_ptr;
    }

    /// @brief Unwrap the stored reference (const lvalue Option)
    /// @return Const reference to stored object
    /// @note Aborts if None
    [[nodiscard]] const T &unwrap() const noexcept {
        return const_cast<Option *>(this)->unwrap();
    }

    /// @brief Get stored reference or a default
    /// @param default_ref Reference to return if Option is empty
    /// @return Reference to the stored object or default_ref
    [[nodiscard]] constexpr T &unwrapOr(T &default_ref) const noexcept {
        return this->isSome() ? *_ptr : default_ref;
    }

private:
    T *_ptr;

    /// @note called by someRef()
    explicit constexpr Option(T &ref) noexcept :
        _ptr{&ref} {}

    using Self = Option<T &>;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _ptr != nullptr;
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        _ptr = nullptr;
    }
};

/// @brief Optional slice container
/// @tparam T Slice element type
template<typename T> struct Option<Slice<T>> final :

    OptionTag,
    mixin::Invariant<Option<Slice<T>>>,
    mixin::Resettable<Option<Slice<T>>>

{
    friend struct internal::SomeCreator;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept :
        _ptr{nullptr}, _length{is_none_mark} {}

    /// @brief Default constructor
    /// @note Intended for containers
    constexpr Option() noexcept :
        _ptr{nullptr}, _length{is_none_mark} {}

    /// @brief Unwrap the stored slice
    /// @return `Slice<T>` (by value)
    /// @note Aborts if None
    [[nodiscard]] Slice<T> unwrap() const noexcept {
        if (this->isSome()) { return {_ptr, _length}; }
        abort();
    }

    /// @brief Get stored slice or a default
    /// @param default_slice Slice to return if Option is empty
    /// @return The stored slice if Some, otherwise default_slice
    [[nodiscard]] constexpr Slice<T> unwrapOr(Slice<T> default_slice) const noexcept {
        return this->isSome() ? Slice<T>{_ptr, _length} : default_slice;
    }

private:
    static constexpr auto is_none_mark{static_cast<usize>(-1)};

    T *_ptr;
    usize _length;

    /// @note called by some()
    explicit constexpr Option(Slice<T> slice) noexcept :
        _ptr{slice.data()}, _length{slice.length()} {}

    using Self = Option<Slice<T>>;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _length != is_none_mark;
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        _length = is_none_mark;
        _ptr = nullptr;
    }
};

/// @brief Optional function container (move‑only)
/// @tparam R Function return type
/// @tparam Args Function argument types
/// @note Function is move‑only; copy is disabled
template<typename R, typename... Args> struct Option<Function<R(Args...)>> final :

    OptionTag,
    mixin::Invariant<Option<Function<R(Args...)>>>,
    mixin::Resettable<Option<Function<R(Args...)>>>

{
    friend struct internal::SomeCreator;

    using FunctionType = Function<R(Args...)>;

    /// @brief Construct an empty Option (None)
    constexpr Option(NoneType) noexcept :
        _function{} {}

    /// @brief Default constructor
    /// @note Intended for containers
    constexpr Option() noexcept :
        _function{} {}

    /// @brief Move constructor
    /// @param other Source Option (becomes None after move)
    Option(Option &&other) noexcept :
        _function{std::move(other._function)} {}

    /// @brief Destructor - destroys stored function if present
    ~Option() noexcept {
        this->reset();
    }

    /// @brief Move assignment operator
    /// @param other Source Option (becomes None after move)
    /// @return Reference to this
    Option &operator=(Option &&other) noexcept {
        if (this != &other) {
            _function = std::move(other._function);
        }
        return *this;
    }

    /// @brief Get stored function (const lvalue reference)
    /// @return Const reference to stored function
    /// @note Aborts if None
    [[nodiscard]] const FunctionType &unwrap() const & noexcept {
        if (this->isNone()) { abort(); }
        return _function;
    }

    /// @brief Extract stored function (rvalue)
    /// @return Function object (moved out)
    /// @note Aborts if None
    [[nodiscard]] FunctionType unwrap() && noexcept {
        if (this->isNone()) { abort(); }
        auto ret = std::move(_function);
        this->reset();
        return ret;
    }

private:
    FunctionType _function;

    /// @brief Private constructor for Some (called by kf::some)
    explicit Option(auto &&function) noexcept :
        _function{std::forward<decltype(function)>(function)} {}

    using Self = Option<FunctionType>;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _function.isSome();
    }

    KF_IMPL_RESETTABLE(Self);
    void resetImpl() noexcept {
        if (this->isSome()) {
            _function = std::move(FunctionType{});
        }
    }
};

}// namespace kf