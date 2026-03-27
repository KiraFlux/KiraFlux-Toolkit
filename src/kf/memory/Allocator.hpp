// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>

namespace kf::memory {

/// @brief Alias for std::allocator.
/// @tparam T Type of allocated objects.
template<typename T> using Allocator = std::allocator<T>;

}// namespace kf::memory