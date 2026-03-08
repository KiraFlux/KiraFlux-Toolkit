// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/drivers/bus/Bus.hpp"
#include "kf/drivers/bus/iic/Tag.hpp"

namespace kf::drivers::bus::iic {

/// @brief CRTP base class for IIC bus implementations.
/// @tparam Impl Concrete IIC bus implementation (must inherit from this class).
/// @note Inherits from iic::Tag and bus::Bus<Impl>, providing type identification
///       and the standard bus interface. Concrete implementations must define
///       a nested Node type and implement all required bus methods.
template<typename BusImpl, typename NodeImpl, typename ErrorImpl> struct IIC : iic::Tag, Bus<BusImpl, NodeImpl, ErrorImpl> {};

}// namespace kf::drivers::bus::iic
