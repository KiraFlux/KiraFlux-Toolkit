// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <queue>

#include "kf/memory/Deque.hpp"


namespace kf {

/// @brief FIFO (first-in, first-out) queue adapter
/// @tparam T Element type
/// @tparam Container Underlying container type (default: kf::Deque<T>)
/// @note Wrapper around std::queue for platforms with standard library support
template<typename T, typename Container = kf::Deque<T>> using Queue = std::queue<T, Container>;

}