// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/config.hpp"

#if kf_port_has_map

#include <map>

#include "kf/memory/Allocator.hpp"

namespace kf {

/// @brief Associative container that stores key-value pairs with unique keys
/// @tparam K Key type (must be comparable)
/// @tparam V Value type
/// @tparam C Comparison function object type (default: std::less<K>)
/// @tparam A Allocator type (default: Allocator<std::pair<K, V>>)
/// @note Wrapper around std::map for platforms with standard library support
template<typename K, typename V, typename C = std::less<K>, typename A = Allocator<std::pair<K, V>>>
using Map = std::map<K, V, C, A>;

}// namespace kf

#else

#error "No implementation of 'Map' for current arch"

#endif