// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

#include "kf/memory/Allocator.hpp"

namespace kf::memory {

/// @brief Alias for std::vector (dynamic array).
/// @tparam T      Element type.
/// @tparam Alloc  Allocator type (default: kf::memory::Allocator<T>).
template<typename T, typename Alloc = kf::memory::Allocator<T>> using ArrayList = std::vector<T, Alloc>;

}// namespace kf::memory