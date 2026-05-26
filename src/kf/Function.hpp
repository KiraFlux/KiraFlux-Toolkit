// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <cstdlib>
#include <functional>
#include <type_traits>
#include <utility>

#include "kf/mixin/NonCopyable.hpp"

namespace kf {

template<typename Signature, size_t BufferSize = 16, size_t Alignment = alignof(std::max_align_t)>
class Function;

/**
 * @brief Type-erased functor with small buffer optimization.
 * 
 * @tparam R Return type
 * @tparam Args Argument types
 * @tparam BufferSize Size of internal storage for small objects
 * @tparam Alignment Alignment requirement for internal storage
 */
template<typename R, typename... Args, size_t BufferSize, size_t Alignment>
class Function<R(Args...), BufferSize, Alignment> : mixin::NonCopyable {
private:
    struct Base {
        virtual ~Base() noexcept = default;
        virtual R invoke(Args... args) = 0;
        virtual void moveTo(void *dest) noexcept = 0;
        virtual Base *cloneTo(void *dest) const noexcept = 0;
    };

    template<typename F>
    struct Impl final : Base {
        F f;

        // Construct from any callable
        explicit Impl(F &&func) noexcept : f(std::forward<F>(func)) {}

        // Copy/move constructors needed for cloneTo and moveTo
        Impl(const Impl &other) : f(other.f) {}
        Impl(Impl &&other) : f(std::move(other.f)) {}

        R invoke(Args... args) override {
            return std::invoke(f, std::forward<Args>(args)...);
        }

        void moveTo(void *dest) noexcept override {
            new (dest) Impl(std::move(f));
        }

        Base *cloneTo(void *dest) const noexcept override {
            new (dest) Impl(*this);
            return reinterpret_cast<Base *>(dest);
        }
    };

    alignas(Alignment) std::byte _storage[BufferSize]{};
    Base *_func = nullptr;

    void destroy() noexcept {
        if (_func) {
            _func->~Base();
            _func = nullptr;
        }
    }

    void moveFrom(Function &&other) noexcept {
        if (other._func) {
            other._func->moveTo(_storage);
            _func = reinterpret_cast<Base *>(_storage);
            other._func->~Base();
            other._func = nullptr;
        }
    }

    template<typename F>
    void construct(F &&f) {
        static_assert(sizeof(Impl<std::decay_t<F>>) <= BufferSize,
                      "Callable object too large for Function buffer");
        static_assert(alignof(Impl<std::decay_t<F>>) <= Alignment,
                      "Callable object requires too strict alignment");
        static_assert(std::is_invocable_r<R, F, Args...>::value,
                      "Callable object is not invocable with given arguments");

        _func = new (_storage) Impl<std::decay_t<F>>(std::forward<F>(f));
    }

public:
    // Default constructor – empty function
    Function() noexcept = default;

    // Construct from nullptr – empty function
    explicit Function(std::nullptr_t) noexcept {}

    // Construct from any callable
    template<typename F, typename = std::enable_if_t<
                             !std::is_same<std::decay_t<F>, Function>::value &&
                             std::is_invocable_r<R, F, Args...>::value>>
    Function(F &&f) {
        construct(std::forward<F>(f));
    }

    // Movable
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

    ~Function() {
        destroy();
    }

    /**
     * @brief Invoke the stored callable with given arguments.
     * 
     * If the function is empty, behaviour depends on return type:
     * - for void: nothing happens
     * - for non-void: returns default-constructed R{}
     */
    template<typename... CallArgs>
    R operator()(CallArgs &&...args) const {
        if constexpr (std::is_void<R>::value) {
            if (_func) {
                _func->invoke(std::forward<CallArgs>(args)...);
            }
        } else {
            if (_func) {
                return _func->invoke(std::forward<CallArgs>(args)...);
            } else {
                abort();
            }
        }
    }

    /// Check if the function holds a callable.
    explicit operator bool() const noexcept {
        return _func != nullptr;
    }

    /// Reset to empty state.
    void reset() noexcept {
        destroy();
    }

    /**
     * @brief Replace the stored callable with a new one.
     */
    template<typename F, typename = std::enable_if_t<std::is_invocable_r<R, F, Args...>::value>>
    void assign(F &&f) {
        destroy();
        construct(std::forward<F>(f));
    }

    /**
     * @brief Swap contents with another Function.
     */
    void swap(Function &other) noexcept {
        alignas(Alignment) std::byte temp[BufferSize];
        Base *temp_func = nullptr;

        if (other._func) {
            other._func->moveTo(temp);
            temp_func = reinterpret_cast<Base *>(temp);
            other._func->~Base();
        }

        if (_func) {
            _func->moveTo(other._storage);
            other._func = reinterpret_cast<Base *>(other._storage);
            _func->~Base();
        } else {
            other._func = nullptr;
        }

        if (temp_func) {
            temp_func->moveTo(_storage);
            _func = reinterpret_cast<Base *>(_storage);
            temp_func->~Base();
        } else {
            _func = nullptr;
        }
    }

    using result_type = R;
};

}// namespace kf