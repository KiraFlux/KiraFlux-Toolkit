// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <deque>

#include "kf/memory/Allocator.hpp"

namespace kf {

template<typename T, typename Alloc = kf::Allocator<T>> using Deque = std::deque<T, Alloc>;

}