// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    Option.hpp
/// @brief   Provides Option<T> for safe optional values, replacing nullable pointers.

#pragma once

#include <cmath>  // std::isnan
#include <cstdlib>// abort
#include <limits>
#include <type_traits>
#include <utility>// std::move std::forward

#include "kf/Function.hpp"
#include "kf/NoneType.hpp"
#include "kf/core.hpp"

#include "kf/mixin/Invariant.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf {

struct OptionTag {};

template<typename T> struct Option;

namespace internal {

template<typename Impl> struct StorageBase :

    mixin::Resettable<Impl>,
    mixin::Invariant<Impl>

{};

struct OptionHelper {

    template<typename R, typename... Args> static constexpr bool function_is_some(Function<R(Args...)> const &f) noexcept {
        return f.isSome();
    }

    template<typename R, typename... Args> static constexpr void function_reset(Function<R(Args...)> &f) noexcept {
        f = std::move(Function<R(Args...)>{});
    }
};

}// namespace internal

template<> struct Option<void> :

    OptionTag,
    internal::StorageBase<Option<void>>

{

    /// @brief None
    constexpr Option() noexcept :
        _is_some{false} {}

    /// @brief None
    constexpr Option(NoneType) noexcept :
        _is_some{false} {}

    /// @brief Some
    explicit constexpr Option(internal::OptionHelper) noexcept :
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
        _is_some = false;
    }
};

template<typename T> [[nodiscard]] constexpr auto some(T &&value) noexcept {
    return Option<std::decay_t<T>>{std::forward<T>(value)};
}

[[nodiscard]] constexpr auto some() noexcept {
    return Option<void>{internal::OptionHelper{}};
}

template<typename T> [[nodiscard]] constexpr auto someRef(T &ref) noexcept {
    return Option<T &>{ref};
}

namespace internal {

// sentinel: can use

template<typename> struct can_use_sentinel : std::false_type {};

template<> struct can_use_sentinel<usize> : std::true_type {};

template<typename T> struct can_use_sentinel<T &> : std::true_type {};

template<float_type T> struct can_use_sentinel<T> : std::true_type {};

template<enum_type T> struct can_use_sentinel<T> : std::true_type {};

// template<typename T> struct can_use_sentinel<Slice<T>> : std::true_type {};

template<typename R, typename... Args> struct can_use_sentinel<Function<R(Args...)>> : std::true_type {};

// usefull constants

constexpr auto usize_sentinel{static_cast<usize>(-1)};

template<enum_type T> constexpr auto enum_sentinel{static_cast<std::underlying_type_t<T>>(-1)};

// is sentinel

constexpr bool is_sentinel(usize value) noexcept {
    return value == usize_sentinel;
}

template<float_type T> constexpr bool is_sentinel(T value) noexcept {
    return std::isnan(value);
}

template<enum_type T> constexpr bool is_sentinel(T value) noexcept {
    return static_cast<std::underlying_type_t<T>>(value) == enum_sentinel<T>;
}

template<typename R, typename... Args> constexpr bool is_sentinel(Function<R(Args...)> const &value) noexcept {
    return not OptionHelper::function_is_some(value);
}

// set sentinel

constexpr void set_sentinel(usize &value) noexcept {
    value = usize_sentinel;
}

template<enum_type T> constexpr void set_sentinel(T &value) noexcept {
    value = static_cast<T>(enum_sentinel<T>);
}

template<float_type T> constexpr void set_sentinel(T &value) noexcept {
    value = std::numeric_limits<T>::quiet_NaN();
}

template<typename R, typename... Args> constexpr void set_sentinel(Function<R(Args...)> &value) noexcept {
    OptionHelper::function_reset(value);
}

// storage implementations

template<typename T> struct SentinelStorage : StorageBase<SentinelStorage<T>> {

    constexpr SentinelStorage() noexcept :
        _value{} {
        set_sentinel(_value);
    }

    constexpr SentinelStorage(T const &value) noexcept :
        _value{value} {}

    constexpr SentinelStorage(T &&value) noexcept :
        _value{std::move(value)} {}

protected:
    [[nodiscard]] constexpr T &get() & noexcept {
        return _value;
    }

    [[nodiscard]] constexpr T const &get() const & noexcept {
        return _value;
    }

    [[nodiscard]] constexpr T &&get() && noexcept {
        return std::move(_value);
    }

private:
    T _value;

    using Self = SentinelStorage<T>;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return not is_sentinel(_value);
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        set_sentinel(_value);
    }
};

template<typename T> struct SentinelStorage<T &> : StorageBase<SentinelStorage<T &>> {

    constexpr SentinelStorage() noexcept :
        _ptr{nullptr} {}

    constexpr SentinelStorage(T &value) noexcept :
        _ptr{&value} {}

protected:
    [[nodiscard]] constexpr T &get() noexcept {
        return *_ptr;
    }

    [[nodiscard]] constexpr T const &get() const noexcept {
        return *_ptr;
    }

private:
    T *_ptr;

    using Self = SentinelStorage<T &>;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _ptr != nullptr;
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        _ptr = nullptr;
    }
};

template<typename T> struct TrivialStorage : StorageBase<TrivialStorage<T>> {

    constexpr TrivialStorage() noexcept :
        _dummy{}, _is_some{false} {}

    constexpr TrivialStorage(T const &value) noexcept :
        _value{value}, _is_some{true} {}

    constexpr TrivialStorage(T &&value) noexcept :
        _value{std::move(value)}, _is_some{true} {}

protected:
    [[nodiscard]] constexpr T &get() & noexcept {
        return _value;
    }

    [[nodiscard]] constexpr T const &get() const & noexcept {
        return _value;
    }

    [[nodiscard]] constexpr T &&get() && noexcept {
        return std::move(_value);
    }

private:
    union {
        T _value;
        char _dummy;
    };
    bool _is_some;

    using Self = TrivialStorage<T>;

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _is_some;
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        _is_some = false;
    }
};

template<typename T> struct BufferedStorage : StorageBase<BufferedStorage<T>> {

private:
    using Self = BufferedStorage<T>;

public:
    // constructors

    /// @brief Construct None
    constexpr BufferedStorage() noexcept :
        _is_some{false} {}

    /// @brief Construct Some
    explicit constexpr BufferedStorage(auto &&value) noexcept
        requires(not std::is_same_v<Self, std::decay_t<decltype(value)>>)
    {
        construct(std::forward<decltype(value)>(value));
    }

    /// @brief Copy
    BufferedStorage(Self const &other) noexcept : _is_some{other._is_some} {
        if (this->isSome()) {
            construct(other.get());
        }
    }

    /// @brief Move
    BufferedStorage(Self &&other) noexcept : _is_some{other._is_some} {
        if (this->isSome()) {
            construct(std::move(other.get()));
            other.reset();
        }
    }

    // assignment

    /// @brief Copy assignment
    /// @param other Source Option
    /// @return Reference to this
    /// @note Copies stored value if any
    BufferedStorage &operator=(Self const &other) noexcept {
        if (this != &other) {
            if (other.isSome()) {
                if (this->isSome()) {
                    this->get() = other.get();
                } else {
                    construct(other.get());
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
    BufferedStorage &operator=(Self &&other) noexcept {
        if (this != &other) {
            this->reset();

            if (other.isSome()) {
                construct(std::move(other.get()));
                other.reset();
            }
        }

        return *this;
    }

    // destructor

    /// @brief Destroy if some
    ~BufferedStorage() noexcept {
        this->reset();
    }

protected:
    [[nodiscard]] constexpr T &get() & noexcept {
        return *reinterpret_cast<T *>(_buffer);
    }

    [[nodiscard]] constexpr T const &get() const & noexcept {
        return *reinterpret_cast<T const *>(_buffer);
    }

    [[nodiscard]] constexpr T &&get() && noexcept {
        return std::move(*reinterpret_cast<T *>(_buffer));
    }

private:
    alignas(T) u8 _buffer[sizeof(T)]{};
    bool _is_some{};

    void construct(auto &&value) noexcept {
        new (static_cast<void *>(_buffer)) T(std::forward<decltype(value)>(value));
        _is_some = true;
    }

    KF_IMPL_INVARIANT(Self);
    constexpr bool isSomeImpl() const noexcept {
        return _is_some;
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        if (_is_some) {
            reinterpret_cast<T *>(_buffer)->~T();
            _is_some = false;
        }
    }
};

template<typename T> using Storage = std::conditional_t<

    can_use_sentinel<T>::value,
    SentinelStorage<T>,
    std::conditional_t<trivial<T>, TrivialStorage<T>, BufferedStorage<T>>

    >;

}// namespace internal

template<typename T> struct Option :

    OptionTag,
    internal::Storage<T>

{
private:
    using Base = internal::Storage<T>;

public:
    // constructors

    /// @brief Construct None implicitly
    constexpr Option() noexcept = default;

    /// @brief Construct from `none`
    constexpr Option(NoneType) noexcept :
        Base{} {}

    /// @brief Construct from value
    template<typename U> explicit constexpr Option(U &&value) noexcept
        requires(not std::is_same_v<std::decay_t<U>, Option>)
        : Base(std::forward<U>(value)) {}

    // unwrap

    [[nodiscard]] constexpr decltype(auto) unwrap() & noexcept {
        check();
        return this->get();
    }

    [[nodiscard]] constexpr decltype(auto) unwrap() const & noexcept {
        check();
        return this->get();
    }

    [[nodiscard]] constexpr decltype(auto) unwrap() && noexcept {
        check();
        return std::move(this->get());
    }

    // unwrap with default

    [[nodiscard]] constexpr decltype(auto) unwrapOr(auto &&default_value) const & noexcept {
        return this->isSome() ? this->get() : std::forward<decltype(default_value)>(default_value);
    }

    [[nodiscard]] constexpr decltype(auto) unwrapOr(auto &&default_value) && noexcept {
        return this->isSome() ? std::move(this->get()) : std::forward<decltype(default_value)>(default_value);
    }

    // mapping

    template<typename F, typename U> using MappedOption = Option<std::invoke_result_t<F, U>>;

    // map (lvalue)
    template<typename F> [[nodiscard]] constexpr auto map(F &&f) & noexcept -> MappedOption<F, T &> {
        if (this->isNone()) {
            return none;
        }

        if constexpr (std::is_void_v<std::invoke_result_t<F, T &>>) {
            f(this->get());
            return some();
        } else {
            return some(f(this->get()));
        }
    }

    // map (const lvalue)
    template<typename F> [[nodiscard]] constexpr auto map(F &&f) const & noexcept -> MappedOption<F, T const &> {
        return const_cast<Option *>(this)->map(f);
    }

    // map (rvalue)
    template<typename F> [[nodiscard]] constexpr auto map(F &&f) && noexcept -> MappedOption<F, T &&> {
        if (this->isNone()) {
            return none;
        }

        if constexpr (std::is_void_v<std::invoke_result_t<F, T &&>>) {
            f(std::move(this->get()));
            return some();
        } else {
            return some(f(std::move(this->get())));
        }
    }

private:
    constexpr void check() const noexcept {
        if (this->isNone()) {
            abort();
        }
    }
};

}// namespace kf