// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#ifdef ARDUINO
#include <Wire.h>
#endif

#include "kf/Result.hpp"
#include "kf/StringView.hpp"
#include "kf/bus/Bus.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/Representable.hpp"
#include "kf/primitives.hpp"
#include "kf/units.hpp"

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
        PinConfigFailed,       ///< Setting SDA/SCL pins failed (Wire.setPins() returned false)
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
            CASE_RETURN(IicError::PinConfigFailed);
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

/// @brief I2C node implementation that adapts Arduino TwoWire to the library's BinaryReadable/BinaryWritable interfaces.
/// @tparam I The bus implementation type (I2C).
/// @note This class is movable but not copyable. It holds a reference to the underlying TwoWire instance
///       and manages the I2C address and transaction state. All I/O operations are blocking.
///       The node is created via `I2C::createNode()` and must remain valid while the bus exists.
template<typename I> struct IicNode :

    bus::BusNode<IicNode<I>, IicError>,
    ::kf::mixin::Configured<IicNodeConfig>

{
    using BusImpl = I;
    using Error = IicError;

    using Self = IicNode<BusImpl>;

    /// @brief Configuration for an Arduino Wire I2C node.
    using Config = IicNodeConfig;

    explicit IicNode(BusImpl &bus, Config const &config) noexcept :
        mixin::Configured<Config>{config}
#ifdef ARDUINO

        ,
        _wire{bus._wire}
#endif

    {
    }

private:
#ifdef ARDUINO
    TwoWire &_wire;
#endif

    KF_IMPL_BUS_NODE(Self, Error);

    using WriteResult = Result<void, Error>;

    // interface impl

    auto readBufferImpl(Slice<u8> buffer) noexcept -> Result<Slice<u8 const>, Error> {
#ifdef ARDUINO
        usize const received = request(buffer.length());

        if (received == 0) {
            return Error::create(Error::Timeout);
        }

        readBytesUnchecked(buffer.data(), received);

        return ok(Slice<u8 const>{buffer.data(), received});
#else
        return Error::create(Error::Unknown);
#endif
    }

    template<typename T> auto readPacketImpl() noexcept -> Result<T, Error> {
#ifdef ARDUINO
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
#else
        return Error::create(Error::Unknown);
#endif
    }

    WriteResult writeBufferImpl(Slice<u8 const> buffer) noexcept {
#ifdef ARDUINO
        beginTransmission();
        usize const written = writeBytes(buffer.data(), buffer.length());
        return endTransmission(written, buffer.length());
#else
        return ok();
#endif
    }

    WriteResult writePacketImpl(auto const &packet) noexcept {
#ifdef ARDUINO
        beginTransmission();
        usize const written = writePacketUnchecked(packet);
        return endTransmission(written, sizeof(decltype(packet)));
#else
        return ok();
#endif
    }

    WriteResult writeMixedImpl(auto const &header, Slice<u8 const> buffer) noexcept {
#ifdef ARDUINO
        beginTransmission();
        usize const header_written = writePacketUnchecked(header);
        usize const buffer_written = writeBytes(buffer.data(), buffer.length());
        return endTransmission(header_written + buffer_written, sizeof(decltype(header)) + buffer.length());
#else
        return ok();
#endif
    }

#ifdef ARDUINO

    /// @brief Request `requested` bytes from the I2C device.
    /// @return Number of bytes actually available.
    [[nodiscard]] usize request(usize requested) noexcept {
        return _wire.requestFrom(static_cast<int>(this->config().address), static_cast<int>(requested));
    }

    /// @brief Read raw bytes from the internal Wire buffer after a successful request.
    /// @warning Assumes that exactly `length` bytes are available; call only after checking request() result.
    void readBytesUnchecked(u8 *buffer, usize length) noexcept {
        (void) _wire.readBytes(buffer, length);
    }

    /// @brief Discard any remaining bytes in the receive buffer (flush).
    /// @note Used after a partial read or error to prepare for the next transaction.
    void discardReceiveBuffer() noexcept {
        auto const to_discard = _wire.available();
        for (auto i = 0; i < to_discard; i += 1) {
            (void) _wire.read();
        }
    }

    /// @brief Begin an I2C transmission (send START condition).
    /// @note Must be called before writing any data.
    void beginTransmission() noexcept {
        _wire.beginTransmission(this->config().address);
    }

    /// @brief Write raw bytes to the I2C device (must be between begin/endTransmission).
    /// @return number of bytes actually placed in the internal transmit buffer (may be less than `length` if buffer full).
    [[nodiscard]] usize writeBytes(u8 const *buffer, usize length) noexcept {
        return _wire.write(buffer, length);
    }

    /// @brief End the transmission (send STOP) and check for errors.
    /// @param written Number of bytes successfully written in this transaction.
    /// @param to_write Total number of bytes that were intended to be written.
    /// @return Success or specific I2C error.
    /// @note Possible errors: AddressNack, DataNack, Timeout, BufferTooLong, Unknown.
    [[nodiscard]] WriteResult endTransmission(usize written, usize to_write) noexcept {
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

    /// @brief Write a single‑byte packet without checking (used internally for small writes).
    [[nodiscard]] usize writePacketUnchecked(u8 packet) noexcept {
        return _wire.write(packet);
    }

    /// @brief Write a multi‑byte packet without checking (used internally).
    [[nodiscard]] usize writePacketUnchecked(auto const &packet) noexcept {
        return writeBytes(reinterpret_cast<u8 const *>(&packet), sizeof(decltype(packet)));
    }

#endif
};

#ifdef ARDUINO
struct IicBusBase :

    bus::Bus<IicBusBase, IicNode<IicBusBase>, IicError>,
    mixin::Configured<IicBusConfig>

{
    friend IicNode<IicBusBase>;

    explicit IicBusBase(IicBusConfig const &config, usize i2c_bus_num) noexcept :
        mixin::Configured<IicBusConfig>{config},
        _wire{static_cast<u8>(i2c_bus_num)} {
    }

private:
    TwoWire _wire;

    KF_IMPL_BUS(IicBusBase, IicError);

    auto initImpl() noexcept -> Result<void, IicError> {
        if (not this->config().hasDefaultPins()) {
            if (not _wire.setPins(static_cast<int>(this->config().gpio_num_sda), static_cast<int>(this->config().gpio_num_scl))) {
                return IicError::create(IicError::PinConfigFailed);
            }
        }

        if (not _wire.begin()) {
            return IicError::create(IicError::BeginFailed);
        }

        if (not this->config().hasDefaultClock()) {
            if (not _wire.setClock(this->config().clock_hz)) {
                return IicError::create(IicError::ClockConfigFailed);
            }
        }

        if (not this->config().hasDefaultTimeout()) {
            _wire.setTimeOut(static_cast<u16>(this->config().timeout));// TwoWire::setTimeout
        }

        if (not this->config().hasDefaultBufferSize()) {
            if (_wire.setBufferSize(this->config().buffer_size) != this->config().buffer_size) {
                return IicError::create(IicError::BufferSizeConfigFailed);
            }
        }

        return ok();
    }

    void quitImpl() noexcept {
        (void) _wire.end();// just ignore
    }
};
#else
struct IicBusBase :

    bus::Bus<IicBusBase, IicNode<IicBusBase>, IicError>,
    mixin::Configured<IicBusConfig>

{
    friend IicNode<IicBusBase>;

    explicit IicBusBase(IicBusConfig const &config, usize i2c_bus_num) noexcept :
        mixin::Configured<IicBusConfig>{config} {}

private:
    KF_IMPL_BUS(IicBusBase, IicError);

    auto initImpl() noexcept -> Result<void, IicError> {
        return ok();
    }

    void quitImpl() noexcept {}
};
#endif

}// namespace kf::internal

namespace kf::bus {

struct I2C : internal::IicBusBase {
    using Config = internal::IicBusConfig;
    using Error = internal::IicError;
    using Node = internal::IicNode<internal::IicBusBase>;

    using internal::IicBusBase::IicBusBase;
};

}// namespace kf::bus

#undef CASE_RETURN
