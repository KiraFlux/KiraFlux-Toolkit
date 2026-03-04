// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <memory>


namespace kf::memory {

template<typename T> using Allocator = std::allocator<T>;

}// namespace kf