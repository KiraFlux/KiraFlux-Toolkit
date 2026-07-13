// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/bus/Bus.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::bus {

struct SpiNodeTag {};

template<typename Impl, typename ErrorImpl> struct SpiNode :

    SpiNodeTag,
    BusNode<Impl, ErrorImpl>,
    mixin::Initable<Impl, void()>

{};

struct SpiBusTag {};

/// @brief CRTP base class for SPI bus implementations.
/// @tparam Impl Concrete bus implementation (must inherit from this class).
template<typename Impl, typename NodeImpl, typename ErrorImpl> struct SPI :

    SpiBusTag,
    Bus<Impl, NodeImpl, ErrorImpl>

{
    KF_CHECK_IMPL(NodeImpl, ::kf::bus::SpiNodeTag);
};

}// namespace kf::bus