// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/Array.hpp"
#include "kf/primitives.hpp"

namespace kf::network {

/// @brief MAC address type
using MacAddress = memory::Array<u8, 6>;

}// namespace kf::network