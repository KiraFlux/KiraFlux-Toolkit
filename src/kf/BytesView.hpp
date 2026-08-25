// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file BytesView.hpp

#pragma once

#include "kf/Slice.hpp"
#include "kf/core.hpp"

namespace kf {

/// @brief Alias for Readonly slice of bytes
using BytesView = Slice<u8 const>;

}// namespace kf