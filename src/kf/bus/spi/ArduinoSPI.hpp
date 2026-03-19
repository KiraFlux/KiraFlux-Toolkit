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

#include "kf/bus/spi/SPI.hpp"

namespace kf::bus::spi {

namespace arduino::internal {

/// Dummy. Arduino SPI cannot provide information about errors
enum class Error : u8 {};

/// @brief SPI node implementation that adapts Arduino SPIClass to the library's Readable/Writable interfaces.
/// @tparam I The bus implementation type (ArduinoSPI).
/// @note This class is movable but not copyable. It manages a dedicated chip select pin and SPI settings.
///       Each transaction begins with CS active and applies the stored SPI configuration.
///       The node is created via ArduinoSPI::createNode() and must outlive the bus.
template<typename I> struct ArduinoNode : Node<ArduinoNode<I>>, memory::io::Readable<ArduinoNode<I>, Error>, memory::io::Writable<ArduinoNode<I>, Error> {
    using BusImpl = I;

    /// @brief Bit order for SPI transfers.
    struct Config {

        /// @brief Bit order for SPI transfers.
        enum class BitOrder : u8 {
            LeastSignificant = SPI_LSBFIRST,

            /// @note default for most SPI devices
            MostSignificant = SPI_MSBFIRST,
        };

        /// @brief SPI mode bits (CPOL and CPHA).
        /// @note Standard SPI modes:
        ///       Mode 0: PolarityBit = 0, PhaseBit = 0
        ///       Mode 1: PolarityBit = 0, PhaseBit = 1
        ///       Mode 2: PolarityBit = 1, PhaseBit = 0
        ///       Mode 3: PolarityBit = 1, PhaseBit = 1
        enum ClockBits : u8 {
            None = 0,

            /// Clock phase (CPHA)
            /// 0 = sample on leading (first) clock edge
            /// 1 = sample on trailing (second) edge
            PhaseBit = 0b01,

            /// Clock polarity (CPOL)
            /// 0 = clock idle low
            /// 1 = clock idle high
            PolarityBit = 0b10,
        };

        u32 clock_hz;// desired SPI clock frequency
        u8 pin_cs;   // software CS pin
        BitOrder bit_order;
        ClockBits clock_bits;

        constexpr explicit Config(
            gpio_num_t chip_select_pin,
            u32 clock_hz,
            BitOrder bit_order = BitOrder::MostSignificant,
            ClockBits clock_bits = ClockBits::None) noexcept :
            clock_hz{clock_hz}, pin_cs{static_cast<u8>(chip_select_pin)}, bit_order{bit_order}, clock_bits{clock_bits} {}

        constexpr SPISettings toArduinoSPISettings() const noexcept {
            return {clock_hz, static_cast<u8>(bit_order), static_cast<u8>(clock_bits)};
        }
    };

    explicit ArduinoNode(BusImpl &bus, const Config &config) noexcept : _spi{bus._spi}, _config{config} {}

private:
    SPIClass &_spi;
    const Config &_config;

    /// @brief Control the chip select line (active low).
    /// @param selected true to pull CS low (select device), false to release.
    void chipSelected(bool selected) noexcept {
        digitalWrite(_config.pin_cs, selected ? LOW : HIGH);
    }

    /// @brief Begin an SPI transaction: pull CS low and apply the stored SPI settings.
    /// @note Must be called before any data transfer.
    void beginTransaction() noexcept {
        chipSelected(true);
        _spi.beginTransaction(_config.toArduinoSPISettings());
    }

    /// @brief End an SPI transaction: pull CS high.
    void endTransaction() noexcept {
        _spi.endTransaction();
        chipSelected(false);
    }

    // Initable impl
    friend struct kf::mixin::Initable<ArduinoNode<I>, void>;

    void initImpl() noexcept {
        pinMode(_config.pin_cs, OUTPUT);
        digitalWrite(_config.pin_cs, HIGH);
    }

    // Readable impl
    friend struct kf::memory::io::Readable<ArduinoNode<I>, Error>;

    /// @brief Read `length` bytes from the device while sending zeros (full‑duplex).
    void readBytes(u8 *buffer, usize length) noexcept {
        _spi.transferBytes(nullptr, buffer, length);
    }

    template<typename> static constexpr bool always_false{false};

    /// @brief Read a 1/2/4‑byte packet using dedicated SPI transfer functions (faster than generic loop).
    /// @tparam T Must be exactly 1, 2 or 4 bytes.
    template<typename T> T readPacketUnchecked() noexcept {
        constexpr usize to_read = sizeof(T);

        if constexpr (to_read == sizeof(u8)) {
            return static_cast<T>(_spi.transfer(0));
        } else if constexpr (to_read == sizeof(u16)) {
            return static_cast<T>(_spi.transfer16(0));
        } else if constexpr (to_read == sizeof(u32)) {
            return static_cast<T>(_spi.transfer32(0));
        } else {
            static_assert(always_false<T>, "readPacketUnchecked supports only 1,2,4 byte types");
        }
    }

    // impl

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
    friend struct kf::memory::io::Writable<ArduinoNode<I>, Error>;

    /// @brief Write raw bytes to the device (simplex).
    void writeBytes(const u8 *buffer, usize length) noexcept {
        _spi.transferBytes(buffer, nullptr, length);
    }

    /// @brief Write a single‑byte packet (optimised).
    void writePacketUnchecked(u8 packet) noexcept { _spi.write(packet); }

    /// @brief Write a 2‑byte packet (optimised).
    void writePacketUnchecked(u16 packet) noexcept { _spi.write16(packet); }

    /// @brief Write a 4‑byte packet (optimised).
    void writePacketUnchecked(u32 packet) noexcept { _spi.write32(packet); }

    /// @brief Write a packet of arbitrary size (generic fallback).
    template<typename T> void writePacketUnchecked(T &&packet) noexcept { writeBytes(reinterpret_cast<const u8 *>(&packet), sizeof(T)); }

    // impl

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

struct ArduinoSPI : public spi::SPI<ArduinoSPI, arduino::internal::ArduinoNode<ArduinoSPI>, arduino::internal::Error> {
    using Error = arduino::internal::Error;
    using Node = arduino::internal::ArduinoNode<ArduinoSPI>;

    friend Node;

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

    // impl
    
    friend struct kf::mixin::Initable<ArduinoSPI, Result<void, Error>>;

    Result<void, Error> initImpl() noexcept {
        if (_config.hasDefaultPins()) {
            _spi.begin();
        } else {
            _spi.begin(_config.pin_sck, _config.pin_miso, _config.pin_mosi);
        }
        return {};
    }

    friend struct kf::mixin::Quitable<ArduinoSPI>;

    void quitImpl() noexcept { _spi.end(); }
};

}// namespace kf::bus::spi
