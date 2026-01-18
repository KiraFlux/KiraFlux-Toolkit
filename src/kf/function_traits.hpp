#pragma once

#include "type_traits.hpp"

namespace kf {

template<typename T> struct function_traits;

template<typename R, typename... Args> struct function_traits<R (*)(Args...)> {
    using type = R(Args...);
    using func_ptr_t = R (*)(Args...);
    using return_type = R;
};

template<typename R, typename... Args> struct function_traits<R(Args...)> {
    using type = R(Args...);
    using func_ptr_t = R (*)(Args...);
    using return_type = R;
};

template<typename F> struct function_traits {
private:
    using callable_traits = function_traits<decltype(&F::operator())>;

public:
    using type = typename callable_traits::type;
    using func_ptr_t = typename callable_traits::func_ptr_t;
    using return_type = typename callable_traits::return_type;
};

template<typename F> struct function_traits<F &> : function_traits<F> {};

template<typename F> struct function_traits<F &&> : function_traits<F> {};

}// namespace kf