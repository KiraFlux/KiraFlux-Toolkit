// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <deque>

#include "kf/memory/Allocator.hpp"

namespace kf::memory {

template<typename T, typename Alloc = kf::memory::Allocator<T>> using Deque = std::deque<T, Alloc>;

}