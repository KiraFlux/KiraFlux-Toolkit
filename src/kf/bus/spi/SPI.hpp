// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/meta/type_check.hpp"

#include "kf/bus/Bus.hpp"
#include "kf/bus/spi/Tag.hpp"
#include "kf/mixin/Initable.hpp"

namespace kf::bus::spi {

struct NodeTag {};
template<typename Impl> struct Node : NodeTag, mixin::Initable<Impl, void> {};

/// @brief CRTP base class for SPI bus implementations.
/// @tparam Impl Concrete bus implementation (must inherit from this class).
template<typename Impl, typename NodeImpl, typename ErrorImpl> struct SPI : spi::Tag, Bus<Impl, NodeImpl, ErrorImpl> {
    kf_crtp_check(NodeImpl, kf::bus::spi::NodeTag);
};

}// namespace kf::bus::spi