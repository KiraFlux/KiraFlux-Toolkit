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
concept trivial = std::is_trivially_copyable_v<T>;

template<typename E>
concept enum_type = std::is_enum_v<E>;

template<typename T>
concept float_type = std::is_floating_point_v<T>;

}// namespace kf