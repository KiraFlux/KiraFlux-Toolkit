// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <queue>

#include "kf/memory/Deque.hpp"

namespace kf::memory {

/// @brief FIFO (first‑in, first‑out) queue adapter.
/// @tparam T        Element type.
/// @tparam Container Underlying container type (default: kf::memory::Deque<T>).
template<typename T, typename Container = kf::memory::Deque<T>> using Queue = std::queue<T, Container>;

}// namespace kf::memory