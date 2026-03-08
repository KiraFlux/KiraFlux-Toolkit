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

#include "kf/drivers/bus/iic/IIC.hpp"

namespace kf::drivers::bus::iic {

struct ArduinoIIC : IIC<ArduinoIIC> {

    enum class Error : u8 {
        ClockConfigFailed,
        BufferSizeConfigFailed,
        PinConfigFailed,
        BeginFailed,

        AddressNack,  ///< Address not acknowledged
        DataNack,     ///< Data not acknowledged
        Timeout,      ///< Transfer timed out
        BufferTooLong,///< Data too long to fit in transmit buffer
        IncompletePacket,

        Unknown,///< Arduino Wire "Other" error
    };

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

    struct Node : memory::io::Readable<Node>, memory::io::Writable<Node> {
        using Error = ArduinoIIC::Error;

        struct Config {
            u8 address;
        };

        explicit Node(ArduinoIIC &bus, const Config &config) noexcept : _wire{bus._wire}, _config{config} {}

    private:
        const Config &_config;
        TwoWire &_wire;

        // Readable impl
        friend Readable<Node>;

        [[nodiscard]] usize request(usize requested) noexcept {
            return _wire.requestFrom(_config.address, static_cast<int>(requested));
        }

        void readBytesUnchecked(u8 *buffer, usize length) noexcept {
            (void) _wire.readBytes(buffer, length);
        }

        void discardReceiveBuffer() noexcept {
            const auto to_discard = _wire.available();
            for (auto i = 0; i < to_discard; i += 1) {
                (void) _wire.read();
            }
        }

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
        friend Writable<Node>;

        void beginTransmission() noexcept { _wire.beginTransmission(_config.address); }

        [[nodiscard]] usize writeBytes(const u8 *buffer, usize lenght) noexcept {
            return _wire.write(buffer, lenght);
        }

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

        [[nodiscard]] usize writePacketUnchecked(u8 packet) noexcept {
            return _wire.write(packet);
        }

        template<typename T> [[nodiscard]] usize writePacketUnchecked(T &&packet) noexcept {
            return writeBytes(reinterpret_cast<const u8 *>(&packet), sizeof(T));
        }

        template<typename T> [[nodiscard]] Result<void, Error> writePacketImpl(T &&packet) noexcept {
            beginTransmission();
            const usize written = writePacketUnchecked(std::forward<T>(packet));
            return endTransmission(written, sizeof(T));
        }
    };

    explicit ArduinoIIC(const Config &config, TwoWire &wire) noexcept : _config{config}, _wire{wire} {}

private:
    const Config &_config;
    TwoWire &_wire;

    //

    // IIC impl
    friend IIC<ArduinoIIC>;

    [[nodiscard]] Result<void, Error> initImpl() noexcept {
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

        if (not _wire.begin()) { return Error::BeginFailed; }

        return {};
    }

    void quitImpl() noexcept {
        (void) _wire.end();// just ignore
    }
};

}// namespace kf::drivers::bus::iic
