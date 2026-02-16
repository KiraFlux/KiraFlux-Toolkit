// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstddef>
#include <functional>// для std::invoke (C++17)
#include <type_traits>
#include <utility>

#include "kf/attributes.hpp"

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
class Function<R(Args...), BufferSize, Alignment> {
private:
    struct Base {
        virtual ~Base() noexcept = default;
        virtual R invoke(Args... args) = 0;
        virtual void move_to(void *dest) noexcept = 0;
        virtual Base *clone_to(void *dest) const noexcept = 0;
    };

    template<typename F>
    struct Impl final : Base {
        F f;

        // Construct from any callable
        explicit Impl(F &&func) noexcept : f(std::forward<F>(func)) {}

        // Copy/move constructors needed for clone_to and move_to
        Impl(const Impl &other) : f(other.f) {}
        Impl(Impl &&other) : f(std::move(other.f)) {}

        R invoke(Args... args) override {
            return std::invoke(f, std::forward<Args>(args)...);
        }

        void move_to(void *dest) noexcept override {
            new (dest) Impl(std::move(f));
        }

        Base *clone_to(void *dest) const noexcept override {
            new (dest) Impl(*this);
            return reinterpret_cast<Base *>(dest);
        }
    };

    alignas(Alignment) std::byte storage_[BufferSize]{};
    Base *func_ = nullptr;

    void destroy() noexcept {
        if (func_) {
            func_->~Base();
            func_ = nullptr;
        }
    }

    void move_from(Function &&other) noexcept {
        if (other.func_) {
            other.func_->move_to(storage_);
            func_ = reinterpret_cast<Base *>(storage_);
            other.func_->~Base();
            other.func_ = nullptr;
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

        func_ = new (storage_) Impl<std::decay_t<F>>(std::forward<F>(f));
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

    // Non-copyable
    Function(const Function &) = delete;
    Function &operator=(const Function &) = delete;

    // Movable
    Function(Function &&other) noexcept {
        move_from(std::move(other));
    }

    Function &operator=(Function &&other) noexcept {
        if (this != &other) {
            destroy();
            move_from(std::move(other));
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
            if (func_) {
                func_->invoke(std::forward<CallArgs>(args)...);
            }
        } else {
            if (func_) {
                return func_->invoke(std::forward<CallArgs>(args)...);
            } else {
                return R{};
            }
        }
    }

    /// Check if the function holds a callable.
    explicit operator bool() const noexcept {
        return func_ != nullptr;
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

        if (other.func_) {
            other.func_->move_to(temp);
            temp_func = reinterpret_cast<Base *>(temp);
            other.func_->~Base();
        }

        if (func_) {
            func_->move_to(other.storage_);
            other.func_ = reinterpret_cast<Base *>(other.storage_);
            func_->~Base();
        } else {
            other.func_ = nullptr;
        }

        if (temp_func) {
            temp_func->move_to(storage_);
            func_ = reinterpret_cast<Base *>(storage_);
            temp_func->~Base();
        } else {
            func_ = nullptr;
        }
    }

    using result_type = R;
};

}// namespace kf