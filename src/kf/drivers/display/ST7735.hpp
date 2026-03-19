// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/bus/spi/SPI.hpp"
#include "kf/gpio/GPIO.hpp"
#include "kf/image/ViewportImage.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/pixel/Rgb565Pixel.hpp"

#include "kf/drivers/display/DisplayDriver.hpp"
#include "kf/drivers/display/Orientation.hpp"

namespace kf::drivers::display {

/// @brief ST7735 TFT display driver for 128x160 RGB565 panels
template<typename Ib, typename Ido> struct ST7735 final : DisplayDriver<ST7735<Ib, Ido>, image::ViewportImage<pixel::Rgb565Pixel, 128, 160>> {
    kf_crtp_check(typename Ib::BusImpl, kf::bus::spi::Tag);
    kf_crtp_check(Ido, kf::gpio::DigitalOutputTag);

    using NodeImpl = Ib;
    using DigitalOutputPinImpl = Ido;
    using PixelImpl = pixel::Rgb565Pixel;

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

    /// @brief Hardware configuration for ST7735
    struct Config {
        Orientation orientation;///< Initial display orientation
    };

    explicit ST7735(const Config &config, NodeImpl &&node, DigitalOutputPinImpl &&pin_data_command, DigitalOutputPinImpl &&pin_reset) noexcept :
        _config{config}, _node{node}, _pin_data_command{pin_data_command}, _pin_reset{pin_reset} {}

private:
    const Config &_config;///< Hardware configuration
    NodeImpl _node;
    DigitalOutputPinImpl _pin_data_command;
    DigitalOutputPinImpl _pin_reset;

    u8 _madctl_base_mode{0};///< Base MADCTL value

    // impl

    using This = ST7735<Ib, Ido>;

    KF_IMPL_INITABLE(This, bool);
    bool initImpl() noexcept {
        _node.init();
        _pin_data_command.init();
        _pin_reset.init();

        this->reset();

        sendCommand(Command::SWRESET);
        delay(150);

        sendCommand(Command::SLPOUT);
        delay(255);

        sendCommand(Command::COLMOD);
        const u8 color_mode{0x05};// 16-bit color (RGB565)
        sendPacket(color_mode);

        (void) this->orientation(_config.orientation);// Ignored becauce always true

        sendCommand(Command::DISPON);
        delay(100);

        return true;// Always returns true (hardware errors not checked)
    }

    friend struct kf::mixin::Resettable<This>;

    /// @brief Hardware reset of the display (pulse RESET pin).
    /// @note Required after power‑up to initialise the internal state machine.
    void resetImpl() const noexcept {
        _pin_reset.write(false);
        delay(10);
        _pin_reset.write(true);
        delay(120);
    }

    friend struct DisplayDriver<This, image::ViewportImage<pixel::Rgb565Pixel, 128, 160>>;

    bool sendImpl() noexcept {
        sendCommand(Command::RAMWR);
        sendBuffer({reinterpret_cast<const u8 *>(this->image().buffer().data()), this->image().size()});
        return true;// Arduino SPI cannot tell anything about errors
    }

    /// @brief Apply orientation transformation (full 6-way support)
    bool setOrientationImpl(Orientation orientation) noexcept {
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

        sendCommand(Command::MADCTL);
        sendPacket(madctl);

        const u8 data_x[4]{0, 0, 0, static_cast<u8>(this->image().maxX())};
        sendCommand(Command::CASET);
        sendPacket(data_x);

        const u8 data_y[4]{0, 0, 0, static_cast<u8>(this->image().maxY())};
        sendCommand(Command::RASET);
        sendPacket(data_y);

        return true;
    }

    // Low-level communication

    void sendBuffer(memory::Slice<const u8> buffer) noexcept {
        _pin_data_command.write(true);
        (void) _node.writeBuffer(buffer);
    }

    template<typename T> void sendPacket(T &&packet) noexcept {
        _pin_data_command.write(true);
        (void) _node.writePacket(std::forward<T>(packet));
    }

    void sendCommand(Command c) noexcept {
        _pin_data_command.write(false);
        (void) _node.writeByte(static_cast<u8>(c));
    }
};

}// namespace kf::drivers::display
