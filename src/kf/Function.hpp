// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>
#include <type_traits>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <functional>

#include "kf/core/attributes.hpp"


namespace kf {

template<typename Signature, size_t BufferSize = 16, size_t Alignment = alignof(std::max_align_t)> struct Function;

template<typename R, typename... Args, size_t BufferSize, size_t Alignment> struct Function<R(Args...), BufferSize, Alignment> {
private:
    struct Base {
        virtual ~Base() noexcept = default;

        virtual R invoke(Args... args) = 0;

        virtual void move_to(void *dest) noexcept = 0;

        virtual Base *clone_to(void *dest) const noexcept = 0;
    };

    template<typename Fn> struct Impl final : Base {
        Fn f;

        explicit Impl(Fn &&func) noexcept:
            f(std::forward<Fn>(func)) {}

        R invoke(Args... args) override {
            return std::invoke(f, std::forward<Args>(args)...);
        }

        void move_to(void *dest) noexcept override {
            new(dest) Impl(std::move(f));
        }

        Base *clone_to(void *dest) const noexcept override {
            new(dest) Impl(f);
            return reinterpret_cast<Base *>(dest);
        }
    };

    alignas(Alignment) std::byte storage[BufferSize]{};
    Base *func = nullptr;

    void destroy() noexcept {
        if (func) {
            func->~Base();
            func = nullptr;
        }
    }

    void move_from(Function &&other) noexcept {
        if (other.func) {
            other.func->move_to(storage);
            func = reinterpret_cast<Base *>(storage);

            other.func->~Base();
            other.func = nullptr;
        }
    }

    template<typename F> void construct(F &&f) {
        static_assert(sizeof(Impl<F>) <= BufferSize, "Callable object too large for Function buffer");
        static_assert(alignof(Impl<F>) <= Alignment, "Callable object requires too strict alignment");
        static_assert(std::is_invocable_r<R, F, Args...>::value, "Callable object is not invocable with given arguments");

        func = new(storage) Impl<F>(std::forward<F>(f));
    }

public:
    Function() noexcept = default;

    explicit Function(std::nullptr_t) noexcept {}

    template<typename F, typename = std::enable_if_t<
        not std::is_same<std::decay_t<F>, Function>::value and
        std::is_invocable_r<R, F, Args...>::value>>
    Function(F &&f) {
        construct(std::forward<F>(f));
    }

    Function(const Function &) = delete;

    Function &operator=(const Function &) = delete;

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

    template<typename... CallArgs> R operator()(CallArgs &&... args) const {
        if constexpr (std::is_void<R>::value) {
            if (func) {
                func->invoke(std::forward<CallArgs>(args)...);
            }
        } else {
            if (func) {
                return func->invoke(std::forward<CallArgs>(args)...);
            } else {
                return R{};
            }
        }
    }

    explicit operator bool() const noexcept {
        return func != nullptr;
    }

    void reset() noexcept {
        destroy();
    }

    template<typename F, typename = std::enable_if_t<std::is_invocable_r<R, F, Args...>::value>> void assign(F &&f) {
        destroy();
        construct(std::forward<F>(f));
    }

    void swap(Function &other) noexcept {
        alignas(Alignment) std::byte temp[BufferSize];
        Base *temp_func = nullptr;

        if (other.func) {
            other.func->move_to(temp);
            temp_func = reinterpret_cast<Base *>(temp);
            other.func->~Base();
        }

        if (func) {
            func->move_to(other.storage);
            other.func = reinterpret_cast<Base *>(other.storage);
            func->~Base();
        } else {
            other.func = nullptr;
        }

        if (temp_func) {
            temp_func->move_to(storage);
            func = reinterpret_cast<Base *>(storage);
            temp_func->~Base();
        } else {
            func = nullptr;
        }
    }

    using result_type = R;
};

template<typename... Args, size_t BufferSize, size_t Alignment> struct Function<void(Args...), BufferSize, Alignment> {
private:
    struct Base {
        virtual ~Base() noexcept = default;

        virtual void invoke(Args... args) = 0;

        virtual void move_to(void *dest) noexcept = 0;
    };

    template<typename Fn> struct Impl final : Base {
        Fn f;

        explicit Impl(Fn &&func) noexcept:
            f(std::forward<Fn>(func)) {}

        void invoke(Args... args) override {
            std::invoke(f, std::forward<Args>(args)...);
        }

        void move_to(void *dest) noexcept override {
            new(dest) Impl(std::move(f));
        }
    };

    alignas(Alignment) std::byte storage[BufferSize]{};
    Base *func = nullptr;

    void destroy() noexcept {
        if (func) {
            func->~Base();
            func = nullptr;
        }
    }

    void move_from(Function &&other) noexcept {
        if (other.func) {
            other.func->move_to(storage);
            func = reinterpret_cast<Base *>(storage);
            other.func->~Base();
            other.func = nullptr;
        }
    }

    template<typename F> void construct(F &&f) {
        static_assert(sizeof(Impl<F>) <= BufferSize, "Callable object too large for Function buffer");
        static_assert(alignof(Impl<F>) <= Alignment, "Callable object requires too strict alignment");
        static_assert(std::is_invocable<F, Args...>::value, "Callable object is not invocable with given arguments");

        func = new(storage) Impl<F>(std::forward<F>(f));
    }

public:
    Function() noexcept = default;

    explicit Function(std::nullptr_t) noexcept {}

    template<typename F, typename = std::enable_if_t<
        not std::is_same<std::decay_t<F>, Function>::value and
        std::is_invocable<F, Args...>::value>>
    Function(F &&f) {
        construct(std::forward<F>(f));
    }

    Function(const Function &) = delete;

    Function &operator=(const Function &) = delete;

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

    template<typename... CallArgs> void operator()(CallArgs &&... args) const {
        if (func) {
            func->invoke(std::forward<CallArgs>(args)...);
        }
    }

    explicit operator bool() const noexcept {
        return func != nullptr;
    }

    void reset() noexcept {
        destroy();
    }

    template<typename F, typename = std::enable_if_t<std::is_invocable<F, Args...>::value>> void assign(F &&f) {
        destroy();
        construct(std::forward<F>(f));
    }

    using result_type = void;
};


} // namespace kf