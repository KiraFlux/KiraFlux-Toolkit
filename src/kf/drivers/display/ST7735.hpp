// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>// for delay

#include "kf/Result.hpp"
#include "kf/bus/spi/SPI.hpp"
#include "kf/gpio/GPIO.hpp"
#include "kf/image/ViewportImage.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/pixel/Rgb565Pixel.hpp"
#include "kf/primitives.hpp"

#include "kf/drivers/display/DisplayDriver.hpp"
#include "kf/drivers/display/Orientation.hpp"

#define TRY(__x__) \
    if (const auto result = __x__; result.isError()) { return result; }

namespace kf::internal {

using ST7735Image = image::ViewportImage<pixel::Rgb565Pixel, 128, 160>;

struct ST7735Config final {
    drivers::display::Orientation init_orientation;
};

}// namespace kf::internal

namespace kf::drivers::display {

/// @brief ST7735 TFT display driver for 128x160 RGB565 panels
/// @tparam N Implementation of SPI bus Node
/// @tparam G Implementation of GPIO with digital input support
template<typename N, typename G> struct ST7735 final :

    DisplayDriver<ST7735<N, G>, internal::ST7735Image, Result<void, typename N::Error>>,
    mixin::Configurable<internal::ST7735Config>

{
    KF_CHECK_IMPL(N, ::kf::bus::spi::SpiNodeTag);
    KF_CHECK_IMPL(G, ::kf::gpio::GPIO::DigitalOutputTag);

    using SpiBusNodeImpl = N;
    using DigitalOutputImpl = G;
    using PixelImpl = typename internal::ST7735Image::PixelImpl;
    using Error = typename SpiBusNodeImpl::Error;
    using SpiOperationResult = Result<void, Error>;

    /// @brief Hardware configuration for ST7735
    using Config = internal::ST7735Config;

    /// @brief Memory Access Control (MADCTL) register bits
    enum MadCtl : u8 {
        RgbMode = 0x00,///< RGB color order
        BgrMode = 0x08,///< BGR color order

        MirrorTranspose = 0x20,///< Transpose image (Swap X and Y axes)
        MirrorX = 0x40,        ///< Horizontal image mirror
        MirrorY = 0x80,        ///< Vertical image mirror
    };

    /// @brief ST7735 command set (partial)
    enum class Command : u8 {
        SWRESET = 0x01,///< Software reset

        SLPIN = 0x10, ///< Enter sleep mode
        SLPOUT = 0x11,///< Exit sleep mode

        INVOFF = 0x20, ///< Disable color inversion
        INVON = 0x21,  ///< Enable color inversion
        DISPOFF = 0x28,///< Turn display off
        DISPON = 0x29, ///< Turn display on
        CASET = 0x2A,  ///< Set column address range
        RASET = 0x2B,  ///< Set row address range
        RAMWR = 0x2C,  ///< Write to display RAM

        MADCTL = 0x36,///< Memory access control
        COLMOD = 0x3A ///< Color mode setting
    };

    explicit ST7735(const Config &config, SpiBusNodeImpl &&node, DigitalOutputImpl &&pin_data_command, DigitalOutputImpl &&pin_reset) noexcept :
        mixin::Configurable<internal::ST7735Config>{config}, _spi_node{std::move(node)}, _pin_data_command{std::move(pin_data_command)}, _pin_reset{std::move(pin_reset)} {}

private:
    SpiBusNodeImpl _spi_node;
    DigitalOutputImpl _pin_data_command;
    DigitalOutputImpl _pin_reset;

    u8 _madctl_base_mode{0};///< Base MADCTL value

    // Low-level communication

    SpiOperationResult sendBuffer(Slice<const u8> buffer) noexcept {
        _pin_data_command.write(true);
        return _spi_node.writeBuffer(buffer);
    }

    template<typename T> SpiOperationResult sendPacket(T &&packet) noexcept {
        _pin_data_command.write(true);
        return _spi_node.writePacket(std::forward<T>(packet));
    }

    SpiOperationResult sendCommand(Command c) noexcept {
        _pin_data_command.write(false);
        return _spi_node.writeByte(static_cast<u8>(c));
    }

    // impl
    using This = ST7735<N, G>;

    KF_IMPL_INITABLE(This, SpiOperationResult);
    SpiOperationResult initImpl() noexcept {
        _spi_node.init();
        _pin_data_command.init();
        _pin_reset.init();

        this->reset();

        TRY(sendCommand(Command::SWRESET));
        delay(150);

        TRY(sendCommand(Command::SLPOUT));
        delay(255);

        TRY(sendCommand(Command::COLMOD));

        constexpr u8 color_mode{0x05};// 16-bit color (RGB565)
        TRY(sendPacket(color_mode));

        TRY(this->orientation(this->config().init_orientation));
        TRY(sendCommand(Command::DISPON));

        delay(100);

        return ok();
    }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() const noexcept {
        // Required after power‑up to initialise the internal state machine.
        _pin_reset.write(false);
        delay(10);
        _pin_reset.write(true);
        delay(120);
    }

    KF_IMPL(DisplayDriver<This, internal::ST7735Image, SpiOperationResult>);
    SpiOperationResult sendImpl() noexcept {
        TRY(sendCommand(Command::RAMWR));
        return sendBuffer({reinterpret_cast<const u8 *>(this->image().buffer().data()), this->image().size()});
    }

    SpiOperationResult setOrientationImpl(Orientation orientation) noexcept {
        // full 6-way support
        constexpr u8 orient_to_transform[]{
            0,                                        // Orientation::Normal
            MadCtl::MirrorX,                          // Orientation::MirrorX
            MadCtl::MirrorY,                          // Orientation::MirrorY
            MadCtl::MirrorX | MadCtl::MirrorY,        // Orientation::Flip
            MadCtl::MirrorX | MadCtl::MirrorTranspose,// Orientation::ClockWise
            MadCtl::MirrorY | MadCtl::MirrorTranspose,// Orientation::CounterClockWise
        };

        const u8 madctl = _madctl_base_mode | orient_to_transform[static_cast<u8>(orientation)];

        this->image().transposed((madctl & MadCtl::MirrorTranspose) != 0);

        const u8 data_x[4]{0, 0, 0, static_cast<u8>(this->image().maxX())};
        const u8 data_y[4]{0, 0, 0, static_cast<u8>(this->image().maxY())};

        TRY(sendCommand(Command::MADCTL))
        TRY(sendPacket(madctl))
        TRY(sendCommand(Command::CASET))
        TRY(sendPacket(data_x))
        TRY(sendCommand(Command::RASET))
        TRY(sendPacket(data_y))
        return ok();
    }
};

}// namespace kf::drivers::display

#undef TRY