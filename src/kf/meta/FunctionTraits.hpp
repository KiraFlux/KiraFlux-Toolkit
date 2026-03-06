// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::meta {

template<typename T> struct FunctionTraits;

template<typename R, typename... Args> struct FunctionTraits<R (*)(Args...)> {
    using type = R(Args...);
    using func_ptr_t = R (*)(Args...);
    using return_type = R;
};

template<typename R, typename... Args> struct FunctionTraits<R(Args...)> {
    using type = R(Args...);
    using func_ptr_t = R (*)(Args...);
    using return_type = R;
};

template<typename F> struct FunctionTraits {
private:
    using callable_traits = FunctionTraits<decltype(&F::operator())>;

public:
    using type = typename callable_traits::type;
    using func_ptr_t = typename callable_traits::func_ptr_t;
    using return_type = typename callable_traits::return_type;
};

template<typename F> struct FunctionTraits<F &> : FunctionTraits<F> {};

template<typename F> struct FunctionTraits<F &&> : FunctionTraits<F> {};

}// namespace kf