// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    bus/I2C.hpp
/// @brief   I2C bus and node abstraction

#pragma once

#ifdef ARDUINO
#include <Wire.h>
#endif

#include "kf/Option.hpp"
#include "kf/Result.hpp"
#include "kf/StringView.hpp"
#include "kf/concepts.hpp"
#include "kf/gpio.hpp"
#include "kf/primitives.hpp"
#include "kf/units.hpp"

#include "kf/mixin/Configured.hpp"
#include "kf/mixin/Representable.hpp"

#include "kf/bus/Bus.hpp"

#define CASE_RETURN(__v) \
    case __v: return #__v

namespace kf::internal {

/// @brief Error codes for I2C operations
/// @note Most errors correspond directly to Arduino Wire library failure conditions
struct IicError : mixin::Representable<IicError, StringView> {

    enum Kind : u8 {

        // Bus errors

        ClockConfigFailed,     ///< Setting I2C clock frequency failed (Wire.setClock() returned false)
        BufferSizeConfigFailed,///< Setting the internal buffer size failed (Wire.setBufferSize() returned different value)
        BeginFailed,           ///< Wire.begin() failed (returned false)

        // Node errors

        AddressNack,     ///< Device did not acknowledge its address after START condition
        DataNack,        ///< Device did not acknowledge a data byte during transmission
        Timeout,         ///< Transaction timed out (exceeded Wire timeout)
        BufferTooLong,   ///< Data to send exceeds the internal Wire transmit buffer size
        IncompletePacket,///< Read operation returned fewer bytes than requested
        Unknown,         ///< Any other unspecified error from Arduino Wire (endTransmission code 4)

    } kind;

    static constexpr auto create(Kind kind) noexcept -> ResultErrorWrapper<IicError> {
        return {IicError{.kind = kind}};
    }

private:
    KF_IMPL_REPRESENTABLE(IicError, StringView);
    constexpr StringView reprImpl() const noexcept {
        switch (kind) {
            CASE_RETURN(IicError::ClockConfigFailed);
            CASE_RETURN(IicError::BufferSizeConfigFailed);
            CASE_RETURN(IicError::BeginFailed);
            CASE_RETURN(IicError::AddressNack);
            CASE_RETURN(IicError::DataNack);
            CASE_RETURN(IicError::Timeout);
            CASE_RETURN(IicError::BufferTooLong);
            CASE_RETURN(IicError::IncompletePacket);

            default:
                CASE_RETURN(IicError::Unknown);
        }
    }
};

using IicWriteResult = Result<void, IicError>;

struct IicNodeConfig final {

    /// @brief 7‑bit I2C device address (usually 0x08–0x77)
    /// @note Wire.h uses 7‑bit format
    u8 address;
};

struct IicBusConfig final {
    Option<gpio::GpioNumber> gpio_num_sda, gpio_num_scl;
    Option<usize> clock_hz, buffer_size, timeout_ms;
};

template<typename Impl> struct IicNodeBase :

    bus::BusNode<Impl, IicError>,
    mixin::Configured<IicNodeConfig>

{
    using mixin::Configured<IicNodeConfig>::Configured;
};

template<typename B> struct IicNodeImpl;

template<typename Impl> struct IicBusBase :

    bus::Bus<Impl, IicNodeImpl<Impl>, IicError>,
    mixin::Configured<IicBusConfig>

{
    using mixin::Configured<IicBusConfig>::Configured;
};

#ifdef ARDUINO

/// @brief I2C node implementation that adapts Arduino TwoWire to the library's BinaryReadable/BinaryWritable interfaces
/// @tparam B The bus implementation type (I2C)
/// @note This class is movable but not copyable. It holds a reference to the underlying TwoWire instance
///       and manages the I2C address and transaction state. All I/O operations are blocking
///       The node is created via `I2C::createNode()` and must remain valid while the bus exists
template<typename B> struct IicNodeImpl : IicNodeBase<IicNodeImpl<B>> {
    using Self = IicNodeImpl<B>;

    using Error = IicError;
    using Config = IicNodeConfig;

    explicit constexpr IicNodeImpl(B &bus, Config const &config) noexcept :
        IicNodeBase<IicNodeImpl>{config}, _wire{bus._wire} {}

private:
    TwoWire &_wire;

    KF_IMPL_BUS_NODE(Self, Error);

    auto readBufferImpl(Slice<u8> buffer) noexcept -> Result<Slice<u8 const>, Error> {
        usize const received = request(buffer.length());

        if (received == 0) {
            return Error::create(Error::Timeout);
        }

        readBytesUnchecked(buffer.data(), received);

        return ok(Slice<u8 const>{buffer.data(), received});
    }

    template<trivial T> auto readPacketImpl() noexcept -> Result<T, Error> {
        constexpr usize requested = sizeof(T);

        usize const received = request(requested);

        if (received == 0) {
            return Error::create(Error::Timeout);
        }

        if (received != requested) {
            discardReceiveBuffer();
            return Error::create(Error::IncompletePacket);
        }

        if constexpr (requested == sizeof(u8)) {
            return ok(static_cast<T>(_wire.read()));
        } else {
            T packet;
            readBytesUnchecked(reinterpret_cast<u8 *>(&packet), requested);
            return ok(packet);
        }
    }

    IicWriteResult writeBufferImpl(Slice<u8 const> buffer) noexcept {
        beginTransmission();
        usize const written = writeBytes(buffer.data(), buffer.length());
        return endTransmission(written, buffer.length());
    }

    IicWriteResult writePacketImpl(trivial auto const &packet) noexcept {
        beginTransmission();
        usize const written = writePacketUnchecked(packet);
        return endTransmission(written, sizeof(decltype(packet)));
    }

    IicWriteResult writeMixedImpl(trivial auto const &header, Slice<u8 const> buffer) noexcept {
        beginTransmission();
        usize const header_written = writePacketUnchecked(header);
        usize const buffer_written = writeBytes(buffer.data(), buffer.length());
        return endTransmission(header_written + buffer_written, sizeof(decltype(header)) + buffer.length());
    }

    /// @brief Request `requested` bytes from the I2C device
    /// @return Number of bytes actually available
    [[nodiscard]] usize request(usize requested) noexcept {
        return _wire.requestFrom(static_cast<int>(this->config().address), static_cast<int>(requested));
    }

    /// @brief Read raw bytes from the internal Wire buffer after a successful request
    /// @warning Assumes that exactly `length` bytes are available; call only after checking request() result
    void readBytesUnchecked(u8 *buffer, usize length) noexcept {
        (void) _wire.readBytes(buffer, length);
    }

    /// @brief Discard any remaining bytes in the receive buffer (flush)
    /// @note Used after a partial read or error to prepare for the next transaction
    void discardReceiveBuffer() noexcept {
        auto const to_discard = _wire.available();
        for (auto i = 0; i < to_discard; i += 1) {
            (void) _wire.read();
        }
    }

    /// @brief Begin an I2C transmission (send START condition)
    /// @note Must be called before writing any data
    void beginTransmission() noexcept {
        _wire.beginTransmission(this->config().address);
    }

    /// @brief Write raw bytes to the I2C device (must be between begin/endTransmission)
    /// @return number of bytes actually placed in the internal transmit buffer (may be less than `length` if buffer full)
    [[nodiscard]] usize writeBytes(u8 const *buffer, usize length) noexcept {
        return _wire.write(buffer, length);
    }

    /// @brief End the transmission (send STOP) and check for errors
    /// @param written Number of bytes successfully written in this transaction
    /// @param to_write Total number of bytes that were intended to be written
    /// @return Success or specific I2C error
    /// @note Possible errors: AddressNack, DataNack, Timeout, BufferTooLong, Unknown
    [[nodiscard]] IicWriteResult endTransmission(usize written, usize to_write) noexcept {
        u8 const code = _wire.endTransmission();

        if (written != to_write) { return Error::create(Error::BufferTooLong); }

        switch (code) {
            case 0: return ok();
            case 1: return Error::create(Error::BufferTooLong);
            case 2: return Error::create(Error::AddressNack);
            case 3: return Error::create(Error::DataNack);
            case 4: return Error::create(Error::Unknown);
            case 5: return Error::create(Error::Timeout);
            default: return Error::create(Error::Unknown);
        }
    }

    /// @brief Write a single‑byte packet without checking (used internally for small writes)
    [[nodiscard]] usize writePacketUnchecked(u8 packet) noexcept {
        return _wire.write(packet);
    }

    /// @brief Write a multi‑byte packet without checking (used internally)
    [[nodiscard]] usize writePacketUnchecked(auto const &packet) noexcept {
        return writeBytes(reinterpret_cast<u8 const *>(&packet), sizeof(decltype(packet)));
    }
};

struct IicBusImpl : IicBusBase<IicBusImpl> {
    using Self = IicBusImpl;

    using Config = IicBusConfig;
    using Error = IicError;
    using Node = IicNodeImpl<IicBusImpl>;

    friend Node;

    explicit IicBusImpl(Config const &config, usize i2c_bus_num) noexcept :
        IicBusBase<Self>{config}, _wire{static_cast<u8>(i2c_bus_num)} {}

private:
    TwoWire _wire;

    [[nodiscard]] static constexpr int uncast(Option<gpio::GpioNumber> g) noexcept {
        if (g.isNone()) {
            return -1;
        } else {
            return static_cast<int>(g.unwrap());
        }
    }

    KF_IMPL_BUS(Self, Error);

    auto initImpl() noexcept -> Result<void, Error> {

        if (not _wire.begin(uncast(this->config().gpio_num_sda), uncast(this->config().gpio_num_scl))) {
            return Error::create(Error::BeginFailed);
        }

        if (not this->config().clock_hz.isSome()) {
            if (not _wire.setClock(this->config().clock_hz.unwrap())) {
                return Error::create(Error::ClockConfigFailed);
            }
        }

        if (not this->config().timeout_ms.isSome()) {
            _wire.setTimeOut(static_cast<u16>(this->config().timeout_ms.unwrap()));// TwoWire::setTimeout
        }

        if (not this->config().buffer_size.isSome()) {
            if (_wire.setBufferSize(this->config().buffer_size.unwrap()) != this->config().buffer_size.unwrap()) {
                return Error::create(Error::BufferSizeConfigFailed);
            }
        }

        return ok();
    }

    void quitImpl() noexcept {
        (void) _wire.end();// just ignore
    }
};

#else

template<typename B> struct IicNodeImpl : IicNodeBase<IicNodeImpl<B>> {
    using Self = IicNodeImpl<B>;

    using Error = IicError;
    using Config = IicNodeConfig;

    explicit constexpr IicNodeImpl(B &bus, Config const &config) noexcept :
        IicNodeBase<Self>{config} {}

private:
    KF_IMPL_BUS_NODE(Self, Error);

    auto readBufferImpl(Slice<u8> buffer) noexcept -> Result<Slice<u8 const>, Error> {
        return Error::create(Error::Unknown);
    }

    template<trivial T> auto readPacketImpl() noexcept -> Result<T, Error> {
        return Error::create(Error::Unknown);
    }

    auto writeBufferImpl(Slice<u8 const> buffer) noexcept -> Result<void, Error> {
        return ok();
    }

    auto writePacketImpl(trivial auto const &packet) noexcept -> Result<void, Error> {
        return ok();
    }

    auto writeMixedImpl(trivial auto const &header, Slice<u8 const> buffer) noexcept -> Result<void, Error> {
        return ok();
    }
};

struct IicBusImpl : IicBusBase<IicBusImpl> {
    using Self = IicBusImpl;

    using Config = IicBusConfig;
    using Error = IicError;
    using Node = IicNodeImpl<IicBusImpl>;

    friend Node;

    explicit constexpr IicBusImpl(Config const &config, usize i2c_bus_num) noexcept :
        IicBusBase<Self>{config} {}

private:
    KF_IMPL_BUS(Self, Error);

    auto initImpl() noexcept -> Result<void, Error> {
        return ok();
    }

    void quitImpl() noexcept {}
};

#endif

}// namespace kf::internal

namespace kf::bus {

using I2C = internal::IicBusImpl;

}// namespace kf::bus

#undef CASE_RETURN
