// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Wire.h>
#include <type_traits>

#include "kf/math.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Readable.hpp"
#include "kf/mixin/Writable.hpp"
#include "kf/primitives.hpp"

#include "kf/bus/IIC.hpp"

namespace kf::internal {

/// @brief Error codes for I2C operations.
/// @note Most errors correspond directly to Arduino Wire library failure conditions.
enum class ArduinoIicError : u8 {
    // Bus errors

    ClockConfigFailed,     ///< Setting I2C clock frequency failed (Wire.setClock() returned false).
    BufferSizeConfigFailed,///< Setting the internal buffer size failed (Wire.setBufferSize() returned different value).
    PinConfigFailed,       ///< Setting SDA/SCL pins failed (Wire.setPins() returned false).
    BeginFailed,           ///< Wire.begin() failed (returned false).

    // Node errors

    AddressNack,     ///< Device did not acknowledge its address after START condition.
    DataNack,        ///< Device did not acknowledge a data byte during transmission.
    Timeout,         ///< Transaction timed out (exceeded Wire timeout).
    BufferTooLong,   ///< Data to send exceeds the internal Wire transmit buffer size.
    IncompletePacket,///< Read operation returned fewer bytes than requested.
    Unknown,         ///< Any other unspecified error from Arduino Wire (endTransmission code 4).
};

struct NodeConfig final {
    /// @brief 7‑bit I2C device address (usually 0x08–0x77)
    /// @note Wire.h uses 7‑bit format
    u8 address;
};

struct ArduinoIicBusConfig final {
    static constexpr u8 gpio_num_nc{static_cast<u8>(GPIO_NUM_NC)};
    static constexpr math::Milliseconds max_timeout{60'000};

    u32 clock_hz;
    math::Milliseconds timeout;
    usize buffer_size;
    u8 gpio_num_sda;
    u8 gpio_num_scl;

    [[nodiscard]] static constexpr ArduinoIicBusConfig create(
        u32 clock_hz = 0,              // 0: use Wire defaults
        math::Milliseconds timeout = 0,// 0: use Wire defaults
        usize buffer_size = 0,         // 0: use Wire defaults
        u8 sda = gpio_num_nc,
        u8 scl = gpio_num_nc) noexcept {
        return ArduinoIicBusConfig{
            .clock_hz = clock_hz,
            .timeout = math::min(timeout, max_timeout),
            .buffer_size = buffer_size,
            .gpio_num_sda = sda,
            .gpio_num_scl = scl,
        };
    }

    constexpr bool hasDefaultPins() const noexcept { return gpio_num_sda == gpio_num_nc and gpio_num_scl == gpio_num_nc; }
    constexpr bool hasDefaultClock() const noexcept { return clock_hz == 0; }
    constexpr bool hasDefaultTimeout() const noexcept { return timeout == 0; }
    constexpr bool hasDefaultBufferSize() const noexcept { return buffer_size == 0; }
};

/// @brief I2C node implementation that adapts Arduino TwoWire to the library's Readable/Writable interfaces.
/// @tparam I The bus implementation type (ArduinoIIC).
/// @note This class is movable but not copyable. It holds a reference to the underlying TwoWire instance
///       and manages the I2C address and transaction state. All I/O operations are blocking.
///       The node is created via `ArduinoIIC::createNode()` and must remain valid while the bus exists.
template<typename I> struct ArduinoIicNode :

    bus::IicNode<ArduinoIicNode<I>, ArduinoIicError>,
    ::kf::mixin::Configurable<NodeConfig>

{
    using BusImpl = I;
    using Error = ArduinoIicError;

    /// @brief Configuration for an Arduino Wire I2C node.
    using Config = NodeConfig;

    explicit ArduinoIicNode(BusImpl &bus, const Config &config) noexcept :
        mixin::Configurable<Config>{config}, _wire{bus._wire} {}

private:
    TwoWire &_wire;

    // impl
    using This = ArduinoIicNode<BusImpl>;

    KF_IMPL_READABLE(This, Error);

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
        const auto to_discard = _wire.available();
        for (auto i = 0; i < to_discard; i += 1) {
            (void) _wire.read();
        }
    }

    // interface impl

    auto readBufferImpl(Slice<u8> buffer) noexcept -> Result<Slice<const u8>, Error> {
        const usize received = request(buffer.length());
        if (received == 0) { return error(Error::Timeout); }

        readBytesUnchecked(buffer.data(), received);
        return ok(Slice<const u8>{buffer.data(), received});
    }

    template<typename T> [[nodiscard]] auto readPacketImpl() noexcept -> Result<T, Error> {
        constexpr usize requested = sizeof(T);
        const usize received = request(requested);
        if (received == 0) { return error(Error::Timeout); }

        if (received != requested) {
            discardReceiveBuffer();
            return error(Error::IncompletePacket);
        }

        if constexpr (requested == sizeof(u8)) {
            return ok(static_cast<T>(_wire.read()));
        } else {
            T packet;
            readBytesUnchecked(reinterpret_cast<u8 *>(&packet), requested);
            return ok(packet);
        }
    }

    //
    using WriteResult = Result<void, Error>;

    /// @brief Begin an I2C transmission (send START condition).
    /// @note Must be called before writing any data.
    void beginTransmission() noexcept {
        _wire.beginTransmission(this->config().address);
    }

    /// @brief Write raw bytes to the I2C device (must be between begin/endTransmission).
    /// @return number of bytes actually placed in the internal transmit buffer (may be less than `length` if buffer full).
    [[nodiscard]] usize writeBytes(const u8 *buffer, usize length) noexcept {
        return _wire.write(buffer, length);
    }

    /// @brief End the transmission (send STOP) and check for errors.
    /// @param written Number of bytes successfully written in this transaction.
    /// @param to_write Total number of bytes that were intended to be written.
    /// @return Success or specific I2C error.
    /// @note Possible errors: AddressNack, DataNack, Timeout, BufferTooLong, Unknown.
    [[nodiscard]] WriteResult endTransmission(usize written, usize to_write) noexcept {
        const u8 code = _wire.endTransmission();

        if (written != to_write) { return error(Error::BufferTooLong); }

        switch (code) {
            case 0: return ok();
            case 1: return error(Error::BufferTooLong);
            case 2: return error(Error::AddressNack);
            case 3: return error(Error::DataNack);
            case 4: return error(Error::Unknown);
            case 5: return error(Error::Timeout);
            default: return error(Error::Unknown);
        }
    }

    /// @brief Write a single‑byte packet without checking (used internally for small writes).
    [[nodiscard]] usize writePacketUnchecked(u8 packet) noexcept {
        return _wire.write(packet);
    }

    /// @brief Write a multi‑byte packet without checking (used internally).
    template<typename T> [[nodiscard]] usize writePacketUnchecked(T &&packet) noexcept {
        return writeBytes(reinterpret_cast<const u8 *>(&packet), sizeof(T));
    }

    // interface impl
    KF_IMPL_WRITABLE(This, WriteResult);

    WriteResult writeBufferImpl(Slice<const u8> buffer) noexcept {
        beginTransmission();
        const usize written = writeBytes(buffer.data(), buffer.length());
        return endTransmission(written, buffer.length());
    }

    template<typename T> WriteResult writePacketImpl(T &&packet) noexcept {
        beginTransmission();
        const usize written = writePacketUnchecked(std::forward<T>(packet));
        return endTransmission(written, sizeof(T));
    }

    template<typename T> WriteResult writeMixedImpl(T &&header, Slice<const u8> buffer) noexcept {
        beginTransmission();
        const usize header_written = writePacketUnchecked(std::forward<T>(header));
        const usize buffer_written = writeBytes(buffer.data(), buffer.length());
        return endTransmission(header_written + buffer_written, sizeof(T) + buffer.length());
    }
};

};// namespace kf::internal

namespace kf::arduino {

struct ArduinoIIC :

    bus::IIC<ArduinoIIC, internal::ArduinoIicNode<ArduinoIIC>, internal::ArduinoIicError>,
    mixin::Configurable<internal::ArduinoIicBusConfig>

{
    using Config = internal::ArduinoIicBusConfig;
    using Error = internal::ArduinoIicError;
    using Node = internal::ArduinoIicNode<ArduinoIIC>;

    friend Node;

    explicit ArduinoIIC(const Config &config, TwoWire &wire) noexcept :
        mixin::Configurable<Config>{config}, _wire{wire} {}

private:
    TwoWire &_wire;

    KF_IMPL_INITABLE(ArduinoIIC, Result<void, Error>());
    auto initImpl() noexcept -> Result<void, Error> {
        if (not this->config().hasDefaultPins()) {
            if (not _wire.setPins(static_cast<int>(this->config().gpio_num_sda), static_cast<int>(this->config().gpio_num_scl))) {
                return error(Error::PinConfigFailed);
            }
        }

        if (not _wire.begin()) {
            return error(Error::BeginFailed);
        }

        if (not this->config().hasDefaultClock()) {
            if (not _wire.setClock(this->config().clock_hz)) {
                return error(Error::ClockConfigFailed);
            }
        }

        if (not this->config().hasDefaultTimeout()) {
            _wire.setTimeOut(static_cast<u16>(this->config().timeout));// TwoWire::setTimeout
        }

        if (not this->config().hasDefaultBufferSize()) {
            if (_wire.setBufferSize(this->config().buffer_size) != this->config().buffer_size) {
                return error(Error::BufferSizeConfigFailed);
            }
        }

        return ok();
    }

    KF_IMPL_QUITABLE(ArduinoIIC);
    void quitImpl() noexcept {
        (void) _wire.end();// just ignore
    }
};

}// namespace kf::arduino