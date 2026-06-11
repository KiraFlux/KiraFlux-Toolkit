// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <map>

#include "kf/memory/Allocator.hpp"

namespace kf::memory {

/// @brief Associative container that stores key‑value pairs with unique keys.
/// @tparam K Key type (must be comparable).
/// @tparam V Value type.
/// @tparam C Comparison function object type (default: std::less<K>).
/// @tparam A Allocator type (default: Allocator<std::pair<K, V>>).
template<typename K, typename V, typename C = std::less<K>, typename A = kf::memory::Allocator<std::pair<K, V>>>
using Map = std::map<K, V, C, A>;

}// namespace kf::memory