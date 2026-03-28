// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <deque>

#include "kf/memory/Allocator.hpp"

namespace kf::memory {

/// @brief Alias for std::deque.
/// @tparam T      Element type.
/// @tparam Alloc  Allocator type (default: kf::memory::Allocator<T>).
template<typename T, typename Alloc = kf::memory::Allocator<T>> using Deque = std::deque<T, Alloc>;

}// namespace kf::memory