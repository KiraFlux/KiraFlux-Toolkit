// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <concepts>
#include <type_traits>

namespace kf {

template<typename Impl, typename Tag>
concept implements = std::derived_from<Impl, Tag>;

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<typename T>
concept trivial = std::is_trivially_copyable_v<T> and std::is_trivially_destructible_v<T>;

template<typename E>
concept enum_type = std::is_enum_v<E>;

template<typename T>
concept float_type = std::is_floating_point_v<T>;

/// @brief Concept for callable objects with a specific signature.
/// @tparam F    The callable type.
/// @tparam Ret  Expected return type.
/// @tparam Args Argument types the callable must accept.
template<typename F, typename Ret, typename... Args>
concept callable = requires(F f, Args... args) {
    { f(args...) } -> std::same_as<Ret>;
};

}// namespace kf