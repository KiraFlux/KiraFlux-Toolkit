// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Wire.h>
#include <type_traits>

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/math/units.hpp"
#include "kf/memory/io/Readable.hpp"
#include "kf/memory/io/Writable.hpp"

#include "kf/bus/iic/IIC.hpp"

namespace kf::bus::iic {

namespace arduino::internal {

/// @brief Error codes for I2C operations.
/// @note Most errors correspond directly to Arduino Wire library failure conditions.
enum class Error : u8 {
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

/// @brief I2C node implementation that adapts Arduino TwoWire to the library's Readable/Writable interfaces.
/// @tparam I The bus implementation type (ArduinoIIC).
/// @note This class is movable but not copyable. It holds a reference to the underlying TwoWire instance
///       and manages the I2C address and transaction state. All I/O operations are blocking.
///       The node is created via `ArduinoIIC::createNode()` and must remain valid while the bus exists.
template<typename I> struct Node : memory::io::Readable<Node<I>, Error>, memory::io::Writable<Node<I>, Error> {
    using BusImpl = I;

    /// @brief Configuration for an Arduino Wire I2C node.
    struct Config {
        u8 address;///< 7‑bit I2C device address (usually 0x08–0x77). Wire.h uses 7‑bit format
    };

    explicit Node(BusImpl &bus, const Config &config) noexcept : _wire{bus._wire}, _config{config} {}

private:
    const Config &_config;
    TwoWire &_wire;

    // Readable impl
    friend struct kf::memory::io::Readable<Node<BusImpl>, Error>;

    /// @brief Request `requested` bytes from the I2C device.
    /// @return Number of bytes actually available.
    [[nodiscard]] usize request(usize requested) noexcept {
        return _wire.requestFrom(static_cast<int>(_config.address), static_cast<int>(requested));
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

    Result<memory::Slice<const u8>, Error> readBufferImpl(memory::Slice<u8> buffer) noexcept {
        const usize received = request(buffer.size());
        if (received == 0) { return {Error::Timeout}; }

        readBytesUnchecked(buffer.data(), received);
        return {memory::Slice<const u8>{buffer.data(), received}};
    }

    template<typename T> [[nodiscard]] Result<T, Error> readPacketImpl() noexcept {
        constexpr usize requested = sizeof(T);
        const usize received = request(requested);
        if (received == 0) { return {Error::Timeout}; }

        if (received != requested) {
            discardReceiveBuffer();
            return {Error::IncompletePacket};
        }

        if constexpr (requested == sizeof(u8)) {
            return {static_cast<T>(_wire.read())};
        } else {
            T packet;
            readBytesUnchecked(reinterpret_cast<u8 *>(&packet), requested);
            return {packet};
        }
    }

    //

    // Writable impl
    friend struct kf::memory::io::Writable<Node<BusImpl>, Error>;

    /// @brief Begin an I2C transmission (send START condition).
    /// @note Must be called before writing any data.
    void beginTransmission() noexcept { _wire.beginTransmission(_config.address); }

    /// @brief Write raw bytes to the I2C device (must be between begin/endTransmission).
    /// @return number of bytes actually placed in the internal transmit buffer (may be less than `length` if buffer full).
    [[nodiscard]] usize writeBytes(const u8 *buffer, usize lenght) noexcept {
        return _wire.write(buffer, lenght);
    }

    /// @brief End the transmission (send STOP) and check for errors.
    /// @param written Number of bytes successfully written in this transaction.
    /// @param to_write Total number of bytes that were intended to be written.
    /// @return Success or specific I2C error.
    /// @note Possible errors: AddressNack, DataNack, Timeout, BufferTooLong, Unknown.
    [[nodiscard]] Result<void, Error> endTransmission(usize written, usize to_write) noexcept {
        const u8 code = _wire.endTransmission();

        if (written != to_write) { return {Error::BufferTooLong}; }

        switch (code) {
            case 0: return {};
            case 1: return {Error::BufferTooLong};
            case 2: return {Error::AddressNack};
            case 3: return {Error::DataNack};
            case 4: return {Error::Unknown};
            case 5: return {Error::Timeout};
            default: return {Error::Unknown};
        }
    }

    [[nodiscard]] Result<void, Error> writeBufferImpl(memory::Slice<const u8> buffer) noexcept {
        beginTransmission();
        const usize written = writeBytes(buffer.data(), buffer.size());
        return endTransmission(written, buffer.size());
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

    template<typename T> [[nodiscard]] Result<void, Error> writePacketImpl(T &&packet) noexcept {
        beginTransmission();
        const usize written = writePacketUnchecked(std::forward<T>(packet));
        return endTransmission(written, sizeof(T));
    }

    template<typename T> [[nodiscard]] Result<void, Error> writeMixedImpl(T &&header, memory::Slice<const u8> buffer) noexcept {
        beginTransmission();
        const usize header_written = writePacketUnchecked(std::forward<T>(header));
        const usize buffer_written = writeBytes(buffer.data(), buffer.size());
        return endTransmission(header_written + buffer_written, sizeof(T) + buffer.size());
    }
};

};// namespace arduino::internal

struct ArduinoIIC : IIC<ArduinoIIC, arduino::internal::Node<ArduinoIIC>, arduino::internal::Error> {
    using Error = arduino::internal::Error;
    using Node = arduino::internal::Node<ArduinoIIC>;

    friend Node;

    struct Config {
        static constexpr u8 pin_default = 0xff;
        static constexpr math::Milliseconds max_timeout{60'000};

        u32 clock_hz;
        math::Milliseconds timeout;
        usize buffer_size;
        u8 pin_sda;
        u8 pin_scl;

        explicit constexpr Config(
            u32 clock_hz = 0,              // 0: use Wire defaults
            math::Milliseconds timeout = 0,// 0: use Wire defaults
            usize buffer_size = 0,         // 0: use Wire defaults
            u8 sda = pin_default,
            u8 scl = pin_default) noexcept : clock_hz{clock_hz}, timeout{kf::min(timeout, max_timeout)}, buffer_size{buffer_size}, pin_sda{sda}, pin_scl{scl} {}

        constexpr bool hasDefaultPins() const noexcept { return pin_sda == pin_default and pin_scl == pin_default; }
        constexpr bool hasDefaultClock() const noexcept { return clock_hz == 0; }
        constexpr bool hasDefaultTimeout() const noexcept { return timeout == 0; }
        constexpr bool hasDefaultBufferSize() const noexcept { return buffer_size == 0; }
    };

    explicit ArduinoIIC(const Config &config, TwoWire &wire) noexcept : _config{config}, _wire{wire} {}

private:
    const Config &_config;
    TwoWire &_wire;

    // impl

    friend struct kf::mixin::Initable<ArduinoIIC, Result<void, Error>>;

    Result<void, Error> initImpl() noexcept {
        if (not _wire.begin()) { return Error::BeginFailed; }

        if (not _config.hasDefaultClock()) {
            if (not _wire.setClock(_config.clock_hz)) { return Error::ClockConfigFailed; }
        }

        if (not _config.hasDefaultTimeout()) {
            _wire.setTimeOut(static_cast<u16>(_config.timeout));// TwoWire::setTimeout
        }

        if (not _config.hasDefaultBufferSize()) {
            if (_wire.setBufferSize(_config.buffer_size) != _config.buffer_size) { return Error::BufferSizeConfigFailed; }
        }

        if (not _config.hasDefaultPins()) {
            if (not _wire.setPins(static_cast<int>(_config.pin_sda), static_cast<int>(_config.pin_scl))) { return Error::PinConfigFailed; }
        }

        return {};
    }

    friend struct kf::mixin::Quitable<ArduinoIIC>;

    void quitImpl() noexcept {
        (void) _wire.end();// just ignore
    }
};

}// namespace kf::bus::iic
