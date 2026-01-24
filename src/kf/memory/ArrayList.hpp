// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <vector>

#include "kf/memory/Allocator.hpp"


namespace kf {

template<typename T, typename Alloc = kf::Allocator<T>> using ArrayList = std::vector<T, Alloc>;

}
