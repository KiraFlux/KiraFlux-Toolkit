// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Result.hpp"
#include "kf/bus/SPI.hpp"
#include "kf/concepts.hpp"
#include "kf/gpio.hpp"
#include "kf/image/ViewportImage.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/pixel/Rgb565Pixel.hpp"
#include "kf/primitives.hpp"
#include "kf/rtos/Task.hpp"

#include "kf/driver/display/DisplayDriver.hpp"
#include "kf/driver/display/Orientation.hpp"

namespace kf::internal {

using ST7735Image = image::ViewportImage<pixel::Rgb565Pixel, 128, 160>;

struct ST7735Config final {
    driver::display::Orientation init_orientation;
};

}// namespace kf::internal

namespace kf::driver::display {

/// @brief ST7735 TFT display driver for 128x160 RGB565 panels
/// @tparam N Implementation of SPI bus Node
/// @tparam G Implementation of GPIO with digital input support
template<implements<bus::SpiNodeTag> N, implements<gpio::DigitalOutputTag> G> struct ST7735 final :

    DisplayDriver<ST7735<N, G>, internal::ST7735Image, Result<void, typename N::Error>>,
    mixin::Configured<internal::ST7735Config>

{

    using SpiBusNodeImpl = N;
    using DigitalOutputImpl = G;
    using PixelImpl = typename internal::ST7735Image::PixelImpl;
    using Error = typename SpiBusNodeImpl::Error;
    using SpiOperationResult = Result<void, Error>;

    using Self = ST7735<N, G>;

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

    explicit constexpr ST7735(const Config &config, SpiBusNodeImpl &&node, DigitalOutputImpl &&gpio_data_command, DigitalOutputImpl &&gpio_reset) noexcept :
        mixin::Configured<internal::ST7735Config>{config}, _spi_node{std::move(node)}, _gpio_data_command{std::move(gpio_data_command)}, _gpio_hardware_reset{std::move(gpio_reset)} {}

private:
    SpiBusNodeImpl _spi_node;
    DigitalOutputImpl _gpio_data_command;
    DigitalOutputImpl _gpio_hardware_reset;

    u8 _madctl_base_mode{0};///< Base MADCTL value

    // Low-level communication

    SpiOperationResult sendBuffer(Slice<const u8> buffer) noexcept {
        _gpio_data_command.write(true);
        return _spi_node.writeBuffer(buffer);
    }

    SpiOperationResult sendPacket(auto const &packet) noexcept {
        _gpio_data_command.write(true);
        return _spi_node.writePacket(std::forward<decltype(packet)>(packet));
    }

    SpiOperationResult sendCommand(Command c) noexcept {
        _gpio_data_command.write(false);
        return _spi_node.writeByte(static_cast<u8>(c));
    }

    KF_IMPL_DISPLAY_DRIVER(Self, internal::ST7735Image, SpiOperationResult);

    SpiOperationResult initImpl() noexcept {
        _spi_node.init();
        _gpio_data_command.init();
        _gpio_hardware_reset.init();

        this->reset();

        KF_TRY(sendCommand(Command::SWRESET));
        rtos::Task::sleep(150);

        KF_TRY(sendCommand(Command::SLPOUT));
        rtos::Task::sleep(255);

        KF_TRY(sendCommand(Command::COLMOD));

        constexpr u8 color_mode{0x05};// 16-bit color (RGB565)
        KF_TRY(sendPacket(color_mode));

        KF_TRY(this->orientation(this->config().init_orientation));
        KF_TRY(sendCommand(Command::DISPON));

        rtos::Task::sleep(100);

        return ok();
    }

    void resetImpl() const noexcept {
        // Required after power‑up to initialise the internal state machine.
        _gpio_hardware_reset.write(false);
        rtos::Task::sleep(10);
        _gpio_hardware_reset.write(true);
        rtos::Task::sleep(120);
    }

    SpiOperationResult sendImpl() noexcept {
        KF_TRY(sendCommand(Command::RAMWR));
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

        KF_TRY(sendCommand(Command::MADCTL));
        KF_TRY(sendPacket(madctl));
        KF_TRY(sendCommand(Command::CASET));
        KF_TRY(sendPacket(data_x));
        KF_TRY(sendCommand(Command::RASET));
        KF_TRY(sendPacket(data_y));
        return ok();
    }
};

}// namespace kf::driver::display