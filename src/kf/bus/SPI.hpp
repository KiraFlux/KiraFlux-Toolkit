// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#ifdef ARDUINO
#include <Arduino.h>
#include <SPI.h>
#endif

#include "kf/Result.hpp"
#include "kf/concepts.hpp"
#include "kf/primitives.hpp"

#include "kf/mixin/Configured.hpp"
#include "kf/mixin/Initable.hpp"

#include "kf/bus/Bus.hpp"

namespace kf::internal {

/// Dummy. Arduino SPI cannot provide information about errors
struct SpiError {};

struct SpiNodeConfig final {

    /// @brief Bit order for SPI transfers
    enum class BitOrder : u8 {
        LeastSignificant = (
#ifdef SPI_LSBFIRST
            SPI_LSBFIRST
#else
            0
#endif
            ),

        /// @note default for most SPI devices
        MostSignificant = (
#ifdef SPI_MSBFIRST
            SPI_MSBFIRST
#else
            1
#endif
            ),
    };

    /// @brief SPI mode bits (CPOL and CPHA)
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

    u32 clock_hz;  // desired SPI clock frequency
    u8 gpio_num_cs;// software CS pin
    BitOrder bit_order;
    ClockBits clock_bits;
};

struct SpiBusConfig final {
    u8 gpio_num_mosi, gpio_num_miso, gpio_num_sck;

    [[nodiscard]] constexpr bool hasDefaultPins() const noexcept {
        constexpr auto gpio_num_nc{static_cast<u8>(-1)};

        return gpio_num_mosi == gpio_num_nc and gpio_num_miso == gpio_num_nc and gpio_num_sck == gpio_num_nc;
    }
};

template<typename Impl> struct SpiNodeBase :

    bus::BusNode<Impl, SpiError>,
    mixin::Initable<Impl, void()>,
    mixin::Configured<SpiNodeConfig>

{
    using mixin::Configured<SpiNodeConfig>::Configured;
};

template<typename B> struct SpiNodeImpl;

template<typename Impl> struct SpiBusBase :

    bus::Bus<Impl, SpiNodeImpl<Impl>, SpiError>,
    mixin::Configured<SpiBusConfig>

{
    using mixin::Configured<SpiBusConfig>::Configured;
};

#ifdef ARDUINO

/// @brief SPI node implementation that adapts Arduino SPIClass to the library's BinaryReadable/BinaryWritable interfaces
/// @tparam B The bus implementation type (SPI)
template<typename B> struct SpiNodeImpl : SpiNodeBase<SpiNodeImpl<B>> {
    using Self = SpiNodeImpl<B>;

    using Error = SpiError;

    using Config = SpiNodeConfig;

    explicit SpiNodeImpl(B &bus, Config const &config) noexcept :
        SpiNodeBase<Self>{config}, _spi{bus._spi} {}

private:
    SPIClass &_spi;

    /// @brief Control the chip select line (active low)
    /// @param selected true to pull CS low (select device), false to release
    void chipSelected(bool selected) noexcept {
        digitalWrite(this->config().gpio_num_cs, selected ? LOW : HIGH);
    }

    /// @brief Begin an SPI transaction: pull CS low and apply the stored SPI settings
    /// @note Must be called before any data transfer
    void beginTransaction() noexcept {
        chipSelected(true);

        _spi.beginTransaction(SPISettings{
            this->config().clock_hz,
            static_cast<u8>(this->config().bit_order),
            static_cast<u8>(this->config().clock_bits),
        });
    }

    /// @brief End an SPI transaction: pull CS high
    void endTransaction() noexcept {
        _spi.endTransaction();
        chipSelected(false);
    }

    /// @brief Read a 1/2/4‑byte packet using dedicated SPI transfer functions (faster than generic loop)
    /// @tparam T Must be exactly 1, 2 or 4 bytes
    template<trivial T> T readPacketUnchecked() noexcept {
        constexpr usize to_read = sizeof(T);

        if constexpr (to_read == sizeof(u8)) {
            return static_cast<T>(_spi.transfer(0));
        } else if constexpr (to_read == sizeof(u16)) {
            return static_cast<T>(_spi.transfer16(0));
        } else if constexpr (to_read == sizeof(u32)) {
            return static_cast<T>(_spi.transfer32(0));
        } else {
            static_assert(not sizeof(T), "readPacketUnchecked supports only 1,2,4 byte types");
        }
    }

    void readBytes(u8 *buffer, usize length) noexcept {
        // Read length bytes from the device while sending zeros (half‑duplex cuz abstraction restrictions)
        _spi.transferBytes(nullptr, buffer, length);
    }

    /// @brief Write raw bytes to the device (simplex)
    void writeBytes(u8 const *buffer, usize length) noexcept {
        _spi.transferBytes(buffer, nullptr, length);
    }

    /// @brief Write a single‑byte packet (optimized)
    void writePacketUnchecked(u8 packet) noexcept {
        _spi.write(packet);
    }

    /// @brief Write a 2‑byte packet (optimized)
    void writePacketUnchecked(u16 packet) noexcept {
        _spi.write16(packet);
    }

    /// @brief Write a 4‑byte packet (optimized)
    void writePacketUnchecked(u32 packet) noexcept {
        _spi.write32(packet);
    }

    /// @brief Write a packet of arbitrary size (generic fallback)
    void writePacketUnchecked(trivial auto const &packet) noexcept {
        writeBytes(reinterpret_cast<u8 const *>(&packet), sizeof(decltype(packet)));
    }

    KF_IMPL_INITABLE(Self, void());
    void initImpl() noexcept {
        pinMode(this->config().gpio_num_cs, OUTPUT);
        digitalWrite(this->config().gpio_num_cs, HIGH);
    }

    KF_IMPL_BUS_NODE(Self, Error);

    auto readBufferImpl(Slice<u8> buffer) noexcept -> Result<Slice<u8 const>, Error> {
        beginTransaction();
        readBytes(buffer.data(), buffer.length());
        endTransaction();
        return ok<Slice<u8 const>>(buffer);
    }

    template<trivial T> auto readPacketImpl() noexcept -> Result<T, Error> {
        constexpr auto to_read = sizeof(T);

        beginTransaction();

        T value;

        if constexpr (to_read <= sizeof(u32)) {
            value = readPacketUnchecked<T>();
        } else {
            readBytes(reinterpret_cast<u8 *>(&value), to_read);
        }

        endTransaction();

        return ok(value);
    }

    using WriteResult = Result<void, Error>;

    WriteResult writeBufferImpl(Slice<u8 const> buffer) noexcept {
        beginTransaction();
        writeBytes(buffer.data(), buffer.length());
        endTransaction();
        return ok();
    }

    WriteResult writePacketImpl(trivial auto const &packet) noexcept {
        beginTransaction();
        writePacketUnchecked(packet);
        endTransaction();
        return ok();
    }

    WriteResult writeMixedImpl(trivial auto const &header, Slice<u8 const> buffer) noexcept {
        beginTransaction();
        writePacketUnchecked(header);
        writeBytes(buffer.data(), buffer.length());
        endTransaction();
        return ok();
    }
};

struct SpiBusImpl : SpiBusBase<SpiBusImpl> {
    using Self = SpiBusImpl;

    using Config = SpiBusConfig;
    using Error = SpiError;
    using Node = SpiNodeImpl<SpiBusImpl>;

    friend Node;

    using SpiBusBase<Self>::SpiBusBase;

private:
    SPIClass _spi{VSPI};

    KF_IMPL_BUS(Self, Error);

    auto initImpl() noexcept -> Result<void, Error> {
        if (this->config().hasDefaultPins()) {
            _spi.begin();
        } else {
            _spi.begin(this->config().gpio_num_sck, this->config().gpio_num_miso, this->config().gpio_num_mosi);
        }
        return ok();
    }

    void quitImpl() noexcept {
        _spi.end();
    }
};

#else

template<typename B> struct SpiNodeImpl : SpiNodeBase<SpiNodeImpl<B>> {
    using Self = SpiNodeImpl<B>;

    using Error = SpiError;
    using Config = SpiNodeConfig;

    explicit SpiNodeImpl(B &bus, Config const &config) noexcept :
        mixin::Configured<Config>{config} {}

private:
    KF_IMPL_INITABLE(Self, void());
    void initImpl() noexcept {}

    KF_IMPL_BUS_NODE(Self, Error);

    auto readBufferImpl(Slice<u8> buffer) noexcept -> Result<Slice<u8 const>, Error> {
        return error(Error{});
    }

    template<trivial T> auto readPacketImpl() noexcept -> Result<T, Error> {
        return error(Error{});
    }

    using WriteResult = Result<void, Error>;

    WriteResult writeBufferImpl(Slice<u8 const> buffer) noexcept {
        return ok();
    }

    WriteResult writePacketImpl(trivial auto const &packet) noexcept {
        return ok();
    }

    WriteResult writeMixedImpl(trivial auto const &header, Slice<u8 const> buffer) noexcept {
        return ok();
    }
};

struct SpiBusImpl : SpiBusBase<SpiBusImpl> {
    using Self = SpiBusImpl;

    using Config = SpiBusConfig;
    using Error = SpiError;
    using Node = SpiNodeImpl<SpiBusImpl>;

    using SpiBusBase<Self>::SpiBusBase;

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

using SPI = internal::SpiBusImpl;

}// namespace kf::bus