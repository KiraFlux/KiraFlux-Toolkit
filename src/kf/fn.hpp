#pragma once

#include "kf/port/autoconfig.hpp"

#if kf_port_has_functional
#include <functional>

namespace kf {
template<typename F> using fn = std::function<F>;
}

#else

#include "stdlib.h"// NOLINT(*-deprecated-headers)

#include "kf/utility.hpp"

namespace kf {
template<typename T> class fn;

template<typename R, typename... Args> class fn<R(Args...)> {
private:
    using func_ptr_t = R (*)(Args...);
    func_ptr_t func_ptr = nullptr;

public:
    fn() noexcept = default;

    fn(nullptr_t) noexcept {}// NOLINT(*-explicit-constructor)

    fn(func_ptr_t f) noexcept :// NOLINT(*-explicit-constructor)
        func_ptr(f) {}

    template<typename F> fn(F f) noexcept :// NOLINT(*-explicit-constructor)
        func_ptr(static_cast<func_ptr_t>(f)) {}

    fn(const fn &other) noexcept = default;

    fn(fn &&other) noexcept = default;

    fn &operator=(nullptr_t) noexcept {
        func_ptr = nullptr;
        return *this;
    }

    fn &operator=(func_ptr_t f) noexcept {
        func_ptr = f;
        return *this;
    }

    fn &operator=(const fn &other) noexcept = default;

    fn &operator=(fn &&other) noexcept = default;

    template<typename F> fn &operator=(F f) noexcept {
        func_ptr = static_cast<func_ptr_t>(f);
        return *this;
    }

    R operator()(Args... args) const {
        if (func_ptr) {
            return func_ptr(args...);
        } else {
            return R();
        }
    }

    explicit operator bool() const noexcept {
        return func_ptr != nullptr;
    }

    bool operator==(nullptr_t) const noexcept {
        return func_ptr == nullptr;
    }

    bool operator!=(nullptr_t) const noexcept {
        return func_ptr != nullptr;
    }

    bool operator==(func_ptr_t f) const noexcept {
        return func_ptr == f;
    }

    bool operator!=(func_ptr_t f) const noexcept {
        return func_ptr != f;
    }
};

}// namespace kf

#endif