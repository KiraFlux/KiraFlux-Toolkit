// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/bus/Bus.hpp"
#include "kf/mixin/Initable.hpp"

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

#define KF_IMPL_SPI_NODE(__impl__, ...)      \
    KF_IMPL_BUS_NODE(__impl__, __VA_ARGS__); \
    KF_IMPL_INITABLE(__impl__, void())

#define KF_IMPL_SPI(...) KF_IMPL_BUS(__VA_ARGS__)
