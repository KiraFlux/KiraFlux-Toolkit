// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <new>
#include <type_traits>
#include <utility>

#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

namespace kf {

template<typename> struct Option;// optional function forward declaration

template<typename> struct Function;

/// @brief Type‑erased callable with small buffer optimisation (2 words). Never empty.
/// @tparam R Return type
/// @tparam Args Argument types
template<typename R, typename... Args> struct Function<R(Args...)> : mixin::NonCopyable {
    friend struct Option<Function<R(Args...)>>;

    explicit Function(std::nullptr_t) = delete;

    template<typename _F> Function(_F &&f) noexcept {
        using Decayed = std::decay_t<_F>;
        static_assert(not std::is_same_v<Decayed, Function>, "Cannot construct Function from another Function (use move)");
        static_assert(sizeof(Impl<Decayed>) <= buffer_size, "Callable object too large for Function buffer");
        static_assert(alignof(Impl<Decayed>) <= alignment, "Callable alignment too strict");
        static_assert(std::is_invocable_r<R, _F, Args...>::value, "Callable not invocable with given arguments");

        _func = new (static_cast<void *>(_storage)) Impl<Decayed>{std::forward<_F>(f)};
    }

    Function(Function &&other) noexcept { moveFrom(std::move(other)); }

    Function &operator=(Function &&other) noexcept {
        if (this != &other) {
            destroy();
            moveFrom(std::move(other));
        }
        return *this;
    }

    ~Function() noexcept { destroy(); }

    /// @brief Invoke the stored callable.
    template<typename... CallArgs> R operator()(CallArgs &&...args) const noexcept {
        if constexpr (std::is_void_v<R>) {
            _func->invoke(std::forward<CallArgs>(args)...);
        } else {
            return _func->invoke(std::forward<CallArgs>(args)...);
        }
    }

private:
    struct Base {
        virtual ~Base() noexcept = default;

        virtual R invoke(Args... args) noexcept = 0;

        virtual void moveTo(void *dest) noexcept = 0;
    };

    template<typename _F> struct Impl final : Base {
        _F f;

        template<typename G> explicit Impl(G &&func) noexcept : f(std::forward<G>(func)) {}

        R invoke(Args... args) noexcept override { return f(std::forward<Args>(args)...); }

        void moveTo(void *dest) noexcept override { new (dest) Impl{std::move(f)}; }
    };

    static constexpr usize buffer_size{2 * sizeof(void *)}, alignment{alignof(std::max_align_t)};

    Base *_func{nullptr};
    alignas(alignment) u8 _storage[buffer_size]{};

    constexpr Function() noexcept {}// access to default contructor only for Option

    void destroy() noexcept {
        if (nullptr != _func) {
            _func->~Base();
            _func = nullptr;
        }
    }

    void moveFrom(Function &&other) noexcept {
        if (nullptr != other._func) {
            other._func->moveTo(static_cast<void *>(_storage));
            _func = reinterpret_cast<Base *>(_storage);
            other._func = nullptr;
        }
    }
};

}// namespace kf