// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/bus/Bus.hpp"

namespace kf::bus {

struct IicNodeTag {};

template<typename Impl, typename ErrorImpl> struct IicNode :

    IicNodeTag,
    BusNode<Impl, ErrorImpl> {};

struct IicBusTag {};

/// @brief CRTP base class for IIC bus implementations.
/// @tparam Impl Concrete IIC bus implementation (must inherit from this class).
/// @note Inherits from IicBusTag and bus::Bus<Impl>, providing type identification
///       and the standard bus interface. Concrete implementations must define
///       a nested Node type and implement all required bus methods.
template<typename BusImpl, typename NodeImpl, typename ErrorImpl> struct IIC :

    IicBusTag,
    Bus<BusImpl, NodeImpl, ErrorImpl> {};

}// namespace kf::bus

#define KF_IMPL_IIC_NODE(...) KF_IMPL_BUS_NODE(__VA_ARGS__)

#define KF_IMPL_IIC(...) KF_IMPL_BUS(__VA_ARGS__)
