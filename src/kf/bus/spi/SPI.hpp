// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/meta/type_check.hpp"

#include "kf/bus/Bus.hpp"
#include "kf/bus/spi/Tag.hpp"
#include "kf/bus/spi/node/Tag.hpp"

namespace kf::bus::spi {

/// @brief CRTP base class for SPI bus implementations.
/// @tparam Impl Concrete bus implementation (must inherit from this class).
template<typename Impl, typename NodeImpl, typename ErrorImpl> struct SPI : spi::Tag, Bus<Impl, NodeImpl, ErrorImpl> {
    kf_crtp_check(NodeImpl, kf::bus::spi::node::Tag);
};

}// namespace kf::bus::spi