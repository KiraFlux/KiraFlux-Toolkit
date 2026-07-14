// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>
#include <new>
#include <type_traits>
#include <utility>

#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

namespace kf {

template<typename> struct Option;// optional function forward declaration

template<typename> struct Function;

/// @brief Type‑erased callable with small buffer optimisation (4 words)
/// @note Never empty.
/// @tparam R Return type
/// @tparam Args Argument types
template<typename R, typename... Args> struct Function<R(Args...)> : mixin::NonCopyable {
    friend struct Option<Function<R(Args...)>>;

    using ReturnType = R;

    explicit Function(std::nullptr_t) = delete;

    Function(auto &&f) noexcept {
        using Decayed = std::decay_t<decltype(f)>;
        static_assert(not std::is_same_v<Decayed, Function>, "Cannot construct Function from another Function (use move)");
        static_assert(sizeof(Impl<Decayed>) <= (buffer_size - 1), "Callable object too large for Function buffer");
        static_assert(alignof(Impl<Decayed>) <= alignment, "Callable alignment too strict");
        static_assert(std::is_invocable_r<R, decltype(f), Args...>::value, "Callable not invocable with given arguments");

        new (static_cast<void *>(_storage)) Impl<Decayed>{std::forward<decltype(f)>(f)};
        isSome(true);
    }

    Function(Function &&other) noexcept {
        moveFrom(std::move(other));
    }

    Function &operator=(Function &&other) noexcept {
        if (this != &other) {
            destroy();
            moveFrom(std::move(other));
        }
        return *this;
    }

    ~Function() noexcept {
        destroy();
    }

    /// @brief Invoke the stored callable
    template<typename... CallArgs> R operator()(CallArgs &&...args) const noexcept {
        if constexpr (std::is_void_v<R>) {
            func().invoke(std::forward<CallArgs>(args)...);
        } else {
            return func().invoke(std::forward<CallArgs>(args)...);
        }
    }

private:
    struct Base {
        virtual ~Base() noexcept = default;

        virtual R invoke(Args... args) const noexcept = 0;

        virtual void moveTo(void *dest) noexcept = 0;
    };

    template<typename _F> struct Impl final : Base {
        _F f;

        explicit Impl(auto &&func) noexcept :
            f(std::forward<decltype(func)>(func)) {}

        R invoke(Args... args) const noexcept override {
            return f(std::forward<Args>(args)...);
        }

        void moveTo(void *dest) noexcept override {
            new (dest) Impl{std::move(f)};
        }
    };

    static constexpr usize buffer_size{4 * sizeof(void *)}, alignment{alignof(std::max_align_t)};

    alignas(alignment) u8 _storage[buffer_size]{};

    // access to default constructor only for Option
    constexpr Function() noexcept {
        isSome(false);
    }

    [[nodiscard]] bool isSome() const noexcept {
        return static_cast<bool>(_storage[buffer_size - 1]);
    }

    void isSome(bool is_isSome) noexcept {
        _storage[buffer_size - 1] = static_cast<u8>(is_isSome);
    }

    [[nodiscard]] Base &func() noexcept {
        return *reinterpret_cast<Base *>(_storage);
    }

    [[nodiscard]] const Base &func() const noexcept {
        return *reinterpret_cast<const Base *>(_storage);
    }

    void destroy() noexcept {
        if (isSome()) {
            func().~Base();
            isSome(false);
        }
    }

    void moveFrom(Function &&other) noexcept {
        if (other.isSome()) {
            other.func().moveTo(static_cast<void *>(_storage));
            isSome(true);
            other.isSome(false);
        }
    }
};

}// namespace kf