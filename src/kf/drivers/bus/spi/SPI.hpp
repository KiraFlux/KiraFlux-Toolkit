// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/drivers/bus/Bus.hpp"
#include "kf/drivers/bus/spi/Tag.hpp"

namespace kf::drivers::bus::spi {

/// @brief CRTP base class for SPI bus implementations.
/// @tparam Impl Concrete bus implementation (must inherit from this class).
template<typename Impl, typename NodeImpl, typename ErrorImpl> struct SPI : spi::Tag, Bus<Impl, NodeImpl, ErrorImpl> {};

}// namespace kf::drivers::bus::spi
