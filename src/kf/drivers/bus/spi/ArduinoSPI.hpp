// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <type_traits>
#include <utility>

#include "kf/Result.hpp"
#include "kf/aliases.hpp"
#include "kf/memory/io/Readable.hpp"
#include "kf/memory/io/Writable.hpp"
#include "kf/memory/io/tags.hpp"

#include "kf/drivers/bus/spi/SPI.hpp"

namespace kf::drivers::bus::spi {

namespace arduino::internal {

enum class Error : u8 {
    BeginFailed,
    BufferTooLong,
};

template<typename I> struct Node : memory::io::Readable<Node<I>, Error>, memory::io::Writable<Node<I>, Error>, {
    using BusImpl = I;

    struct Config {
        enum class BitOrder : u8 {
            LSB = SPI_LSBFIRST,
            MSB = SPI_MSBFIRST,
        };

        enum ClockBits : u8 {
            PhaseBit = 0b01,   // 0 = idle low, 1 = idle high
            PolarityBit = 0b10,// 0 = sample on leading edge
        };

        u32 clock_hz;// desired SPI clock frequency
        u8 pin_cs;   // software CS pin
        BitOrder bit_order;
        ClockBits clock_bits;

        constexpr explicit Config(
            gpio_num_t chip_select_pin,
            u32 clock_hz = 0,// 0: default Arduino SPI clock
            BitOrder bit_order = BitOrder::MSB,
            ClockBits clock_bits = 0) noexcept :
            clock_hz{clock_hz}, pin_cs{static_cast<u8>(chip_select_pin)}, bit_order{bit_order}, clock_bits{clock_bits} {}

        constexpr SPISettings toArduinoSPISettings() const noexcept {
            return {clock_hz, static_cast<u8>(bit_order), static_cast<u8>(clock_bits)};
        }
    };

    explicit Node(BusImpl &bus, const Config &config) noexcept : _spi{bus._spi}, _config{config} {}

private:
    SPIClass &_spi;
    const Config &_config;

    void chipSelected(bool selected) noexcept {
        digitalWrite(_config.pin_cs, selected ? LOW : HIGH);
    }

    void beginTransaction() noexcept {
        chipSelected(true);
        _spi.beginTransaction(_config.toArduinoSPISettings());
    }

    void endTransaction() noexcept {
        _spi.endTransaction();
        chipSelected(false);
    }

    // Readable impl
    friend struct kf::memory::io::Readable<Node<I>, Error>;

    void readBytes(u8 *buffer, usize length) noexcept {
        _spi.transferBytes(nullptr, buffer, length);
    }

    template<typename T> T readPacketUnchecked() noexcept {
        constexpr usize to_read = sizeof(T);

        if constexpr (to_read == sizeof(u8)) {
            return static_cast<T>(_spi.transfer(0));
        } else if constexpr (to_read == sizeof(u16)) {
            return static_cast<T>(_spi.transfer16(0));
        } else if constexpr (to_read == sizeof(u32)) {
            return static_cast<T>(_spi.transfer32(0));
        } else {
            template<typename> constexpr bool always_false{false};
            static_assert(always_false<T>, "readPacketUnchecked supports only 1,2,4 byte types");
        }
    }

    Result<memory::Slice<const u8>, Error> readBufferImpl(memory::Slice<u8> buffer) noexcept {
        beginTransaction();
        readBytes(buffer.data(), buffer.size());
        endTransaction();
        return {buffer};
    }

    template<typename T> Result<T, Error> readPacketImpl() noexcept {
        constexpr usize to_read = sizeof(T);

        beginTransaction();

        T value;

        if constexpr (to_read <= sizeof(u32)) {
            value = readPacketUnchecked<T>();
        } else {
            readBytes(reinterpret_cast<u8 *>(&value), to_read);
        }

        endTransaction();

        return {value};
    }

    // Writable impl
    friend struct kf::memory::io::Writable<Node<I>, Error>;

    void writeBytes(const u8 *buffer, usize length) noexcept {
        _spi.transferBytes(buffer, nullptr, length);
    }

    void writePacketUnchecked(u8 packet) noexcept { _spi.write(packet); }

    void writePacketUnchecked(u16 packet) noexcept { _spi.write16(packet); }

    void writePacketUnchecked(u32 packet) noexcept { _spi.write32(packet); }

    template<typename T> void writePacketUnchecked(T &&packet) noexcept { writeBytes(&packet, sizeof(T)); }

    [[nodiscard]] Result<void, Error> writeBufferImpl(memory::Slice<const u8> buffer) noexcept {
        beginTransaction();
        writeBytes(buffer.data(), buffer.size());
        endTransaction();
        return {};
    }

    template<typename T> [[nodiscard]] Result<void, Error> writePacketImpl(T &&packet) noexcept {
        beginTransaction();
        writePacketUnchecked(std::forward<T>(packet));
        endTransaction();
        return {};
    }

    template<typename T> [[nodiscard]] Result<void, Error> writeMixedImpl(T &&header, memory::Slice<const u8> buffer) noexcept {
        beginTransaction();
        writePacketUnchecked(std::forward<T>(header));
        writeBytes(buffer.data(), buffer.size());
        endTransaction();
        return {};
    }
};

}// namespace arduino::internal

struct ArduinoSPI : public spi::SPI<ArduinoSPI, arduino::internal::Node<ArduinoSPI>, arduino::internal::Error> {
    using Error = arduino::internal::Error;
    using Node = arduino::internal::Node<ArduinoSPI>;

    struct Config {
        using PinIndex = i8;

        static constexpr PinIndex default_pin = static_cast<PinIndex>(GPIO_NUM_NC);

        PinIndex pin_mosi, pin_miso, pin_sck;

        constexpr explicit Config(
            gpio_num_t mosi = GPIO_NUM_NC,
            gpio_num_t miso = GPIO_NUM_NC,
            gpio_num_t sck = GPIO_NUM_NC) noexcept :
            pin_mosi{static_cast<PinIndex>(mosi)},
            pin_miso{static_cast<PinIndex>(miso)},
            pin_sck{static_cast<PinIndex>(sck)} {}

        constexpr bool hasDefaultPins() const noexcept {
            return pin_mosi == default_pin and pin_miso == default_pin and pin_sck == default_pin;
        }
    };

    explicit ArduinoSPI(const Config &config, SPIClass &spi) : _config{config}, _spi{spi} {}

private:
    const Config _config;
    SPIClass &_spi;

    // SPI impl
    friend struct kf::drivers::bus::Bus<ArduinoSPI, Node, Error>;
    friend struct kf::drivers::bus::spi::SPI<ArduinoSPI, Node, Error>;

    [[nodiscard]] Result<void, Error> initImpl() noexcept {
        if (_config.hasDefaultPins()) {
            _spi.begin();
        } else {
            _spi.begin(_config.pin_sck, _config.pin_miso, _config.pin_mosi);
        }
        return {};
    }

    void quitImpl() noexcept { _spi.end(); }
};

}// namespace kf::drivers::bus::spi
