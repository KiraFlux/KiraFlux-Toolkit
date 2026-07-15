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

}// namespace kf