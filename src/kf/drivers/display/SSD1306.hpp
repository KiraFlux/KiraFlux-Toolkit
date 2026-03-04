// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Wire.h>

#include "kf/algorithm.hpp"
#include "kf/aliases.hpp"
#include "kf/drivers/display/DisplayDriver.hpp"
#include "kf/image/StaticImage.hpp"
#include "kf/pixel/MonochromePixel.hpp"

namespace kf::drivers::display {

/// @brief SSD1306 OLED display driver for 128x64 monochrome panels
struct SSD1306 final : DisplayDriver<SSD1306, image::StaticImage<pixel::MonochromePixel, 128, 64>> {
    using PixelImpl = pixel::MonochromePixel;

    struct Config {
        u32 i2c_clock_frequency;
        u8 address;

        explicit Config(u32 clock_frequency, u8 address = 0x3C) noexcept :
            i2c_clock_frequency{clock_frequency}, address{address} {}
    };

private:
    const Config &config;
    TwoWire &wire;

public:
    /// @brief Construct SSD1306 driver instance
    explicit SSD1306(const Config &config, TwoWire &wire) noexcept :
        config{config}, wire{wire} {}

    /// @brief Set display contrast level (0..255)
    /// @returns true - operation success
    [[nodiscard]] bool setContrast(u8 value) const {
        wire.beginTransmission(config.address);
        if (wire.write(CommandMode) != 1) { goto fail; }
        if (wire.write(Contrast) != 1) { goto fail; }
        if (wire.write(value) != 1) { goto fail; }
        if (wire.endTransmission() != 0) { goto fail; }

        return true;
    fail:
        return false;
    }

    /// @brief Enable or disable display power
    /// @returns true - operation success
    [[nodiscard]] bool setPower(bool on) noexcept {
        return sendCommand(on ? DisplayOn : DisplayOff);
    }

    /// @brief Invert display colors
    /// @returns true - operation success
    [[nodiscard]] bool invert(bool invert) noexcept {
        return sendCommand(invert ? InvertDisplay : NormalDisplay);
    }

private:
    // DisplayDriver interface implementation

    /// @brief Initialize display hardware via I2C
    [[nodiscard]] bool initImpl() const noexcept {
        static constexpr u8 init_commands[] = {
            CommandMode,

            // Turn off for safe configuration
            DisplayOff,

            // Clock divider
            ClockDiv,
            0x80,

            // Enable internal charge pump
            ChargePump,
            0x14,

            // Horizontal addressing mode
            AddressingMode,
            Horizontal,

            // Default contrast 127
            Contrast,
            0x7F,

            // VCOM voltage
            SetVcomDetect,
            0x40,

            // Normal orientation
            NormalH,
            NormalV,

            // Turn display on
            DisplayOn,

            // Pin configuration (128x64)
            SetComPins,
            0x12,

            // Multiplex (64 lines)
            SetMultiplex,
            0x3F,
        };

        if (not wire.begin()) { goto fail; }
        if (not wire.setClock(config.i2c_clock_frequency)) { goto fail; }

        wire.beginTransmission(config.address);
        if (wire.write(init_commands, sizeof(init_commands)) != sizeof(init_commands)) { goto fail; }
        if (wire.endTransmission() != 0) { goto fail; }

        return true;
    fail:
        return false;
    }

    /// @brief Transfer software buffer to display via I2C
    [[nodiscard]] bool sendImpl() const noexcept {
        static constexpr auto packet_size = 64u;// Optimal for ESP32 performance

        static constexpr u8 set_area_commands[] = {
            CommandMode,
            // Set full display window
            ColumnAddr,
            0,
            127,
            PageAddr,
            0,
            PixelImpl::template pages<64> - 1,
        };

        auto p = screen_image.buffer().data();
        auto remaining = screen_image.buffer().size();

        wire.beginTransmission(config.address);
        if (wire.write(set_area_commands, sizeof(set_area_commands)) != sizeof(set_area_commands)) { goto fail; }
        if (wire.endTransmission() != 0) { goto fail; }

        while (remaining > 0) {
            const auto chunk = min(packet_size, remaining);

            wire.beginTransmission(config.address);
            if (wire.write(Command::DataMode) != 1) { goto fail; }
            if (wire.write(p, chunk) != chunk) { goto fail; }
            if (wire.endTransmission() != 0) { goto fail; }

            p += chunk;
            remaining -= chunk;
        }

        return true;
    fail:
        return false;
    }

    [[nodiscard]] bool supportOrientation(Orientation orientation) const noexcept {
        switch (orientation) {
            case Orientation::Normal:
            case Orientation::MirrorX:
            case Orientation::MirrorY:
                return true;
            default:
                return false;
        }

        return true;
    }

    [[nodiscard]] bool supportOrientation(Orientation orientation) const noexcept {
        switch (orientation) {
            case Orientation::Normal:
            case Orientation::MirrorX:
            case Orientation::MirrorY:
                return true;
            default:
                return false;
        }
    }

    [[nodiscard]] bool setOrientationImpl(Orientation orientation) noexcept {
        if (not supportOrientation(orientation)) { return false; }

        constexpr auto flip_x = 0b01;
        constexpr auto flip_y = 0b10;
        const auto flags = static_cast<u8>(orientation);

        if (not sendCommand((flags & flip_x) ? FlipH : NormalH)) { goto fail; }
        if (not sendCommand((flags & flip_y) ? FlipV : NormalV)) { goto fail; }

        return true;
    fail:
        return false;
    }

    /// @brief SSD1306 command set
    enum Command : u8 {
        DisplayOff = 0xAE,///< Turn display off
        DisplayOn = 0xAF, ///< Turn display on

        CommandMode = 0x00,   ///< Start command stream
        OneCommandMode = 0x80,///< Single command prefix
        DataMode = 0x40,      ///< Data transmission prefix

        AddressingMode = 0x20,///< Set addressing mode
        Horizontal = 0x00,    ///< Horizontal addressing
        Vertical = 0x01,      ///< Vertical addressing

        NormalV = 0xC8,///< Normal vertical scan direction
        FlipV = 0xC0,  ///< Flipped vertical scan direction
        NormalH = 0xA1,///< Normal horizontal segment remap
        FlipH = 0xA0,  ///< Flipped horizontal segment remap

        Contrast = 0x81,     ///< Set contrast command
        SetComPins = 0xDA,   ///< COM pins hardware configuration
        SetVcomDetect = 0xDB,///< Set VCOMH deselect level
        ClockDiv = 0xD5,     ///< Set display clock divide ratio
        SetMultiplex = 0xA8, ///< Set multiplex ratio
        ColumnAddr = 0x21,   ///< Set column address range
        PageAddr = 0x22,     ///< Set page address range
        ChargePump = 0x8D,   ///< Charge pump setting

        NormalDisplay = 0xA6,///< Normal pixel color (black on white)
        InvertDisplay = 0xA7 ///< Inverted pixel color (white on black)
    };

    /// @brief Send single command to display
    [[nodiscard]] bool sendCommand(Command c) const noexcept {
        wire.beginTransmission(config.address);
        if (wire.write(OneCommandMode) != 1) { goto fail; }
        if (wire.write(static_cast<u8>(c)) != 1) { goto fail; }
        if (wire.endTransmission() != 0) { goto fail; }

        return true;
    fail:
        return false;
    }

    friend Base;// CRTP
};

}// namespace kf
