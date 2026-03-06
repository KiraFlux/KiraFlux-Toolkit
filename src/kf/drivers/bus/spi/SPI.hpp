// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/drivers/bus/Bus.hpp"
#include "kf/drivers/bus/spi/Tag.hpp"

namespace kf::drivers::bus::spi {

/// @brief CRTP base class for SPI bus implementations.
/// @tparam Impl Concrete bus implementation (must inherit from this class).
/// @note Inherits from spi::Tag and bus::Bus<Impl>, providing type identification
///       and the standard bus interface.
///       Concrete implementations must define
///       a nested Node type and implement all required bus methods.
template<typename Impl> struct SPI : spi::Tag, Bus<Impl> {
    using Node = typename Impl::Node;
};

}// namespace kf::drivers::bus::spi
