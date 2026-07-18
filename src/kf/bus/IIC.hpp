// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/bus/Bus.hpp"
#include "kf/concepts.hpp"
#include "kf/primitives.hpp"
#include "kf/units.hpp"

namespace kf::internal {

/// @brief Error codes for I2C operations
/// @note Most errors correspond directly to Arduino Wire library failure conditions
enum class IicError : u8 {

    // Bus errors

    ClockConfigFailed,     ///< Setting I2C clock frequency failed (Wire.setClock() returned false)
    BufferSizeConfigFailed,///< Setting the internal buffer size failed (Wire.setBufferSize() returned different value)
    PinConfigFailed,       ///< Setting SDA/SCL pins failed (Wire.setPins() returned false)
    BeginFailed,           ///< Wire.begin() failed (returned false)

    // Node errors

    AddressNack,     ///< Device did not acknowledge its address after START condition
    DataNack,        ///< Device did not acknowledge a data byte during transmission
    Timeout,         ///< Transaction timed out (exceeded Wire timeout)
    BufferTooLong,   ///< Data to send exceeds the internal Wire transmit buffer size
    IncompletePacket,///< Read operation returned fewer bytes than requested
    Unknown,         ///< Any other unspecified error from Arduino Wire (endTransmission code 4)
};

struct IicNodeConfig final {

    /// @brief 7‑bit I2C device address (usually 0x08–0x77)
    /// @note Wire.h uses 7‑bit format
    u8 address;
};

struct IicBusConfig final {
    static constexpr u8 gpio_num_nc{static_cast<u8>(-1)};

    u8 gpio_num_sda, gpio_num_scl;
    u16 buffer_size;
    u32 clock_hz;
    units::Milliseconds timeout;

    constexpr bool hasDefaultPins() const noexcept { return gpio_num_sda == gpio_num_nc and gpio_num_scl == gpio_num_nc; }

    constexpr bool hasDefaultClock() const noexcept { return clock_hz == 0; }

    constexpr bool hasDefaultTimeout() const noexcept { return timeout == 0; }

    constexpr bool hasDefaultBufferSize() const noexcept { return buffer_size == 0; }
};

}// namespace kf::internal

namespace kf::bus {

struct IicNodeTag {};

template<typename Impl, typename ErrorImpl> struct IicNode :

    IicNodeTag,
    BusNode<Impl, ErrorImpl> {};

struct IicBusTag {};

/// @brief CRTP base class for IIC bus implementations
/// @tparam Impl Concrete IIC bus implementation (must inherit from this class)
/// @note Inherits from IicBusTag and bus::Bus<Impl>, providing type identification
///       and the standard bus interface. Concrete implementations must define
///       a nested Node type and implement all required bus methods
template<typename BusImpl, implements<IicNodeTag> NodeImpl, typename ErrorImpl> struct IIC :

    IicBusTag,
    Bus<BusImpl, NodeImpl, ErrorImpl> {};

}// namespace kf::bus

#define KF_IMPL_IIC_NODE(...) KF_IMPL_BUS_NODE(__VA_ARGS__)

#define KF_IMPL_IIC(...) KF_IMPL_BUS(__VA_ARGS__)
