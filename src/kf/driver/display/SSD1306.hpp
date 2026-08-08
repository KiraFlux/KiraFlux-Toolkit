// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    driver/display/SSD1306.hpp
/// @brief   SSD1306 OLED driver (128×64, I2C, monochrome).

#pragma once

#include "kf/Result.hpp"
#include "kf/bus/I2C.hpp"
#include "kf/image/StaticImage.hpp"
#include "kf/math.hpp"
#include "kf/pixel/MonochromePixel.hpp"
#include "kf/primitives.hpp"

#include "kf/mixin/Configured.hpp"

#include "kf/driver/display/DisplayDriver.hpp"
#include "kf/driver/display/Orientation.hpp"

namespace kf::internal {

using SSD1306Image = image::StaticImage<pixel::MonochromePixel, 128, 64>;

struct SSD1306Config {
    static constexpr u8 default_address{0x3C}, default_chunk_size{64};

    bus::I2C::Node::Config i2c_node;
    u8 chunk_size;
};

}// namespace kf::internal

namespace kf::driver::display {

/// @brief SSD1306 OLED display driver for 128x64 monochrome panels
struct SSD1306 :

    DisplayDriver<SSD1306, internal::SSD1306Image, internal::IicWriteResult>,
    mixin::Configured<internal::SSD1306Config>

{
    using PixelImpl = typename internal::SSD1306Image::PixelImpl;
    using Config = internal::SSD1306Config;

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

    /// @brief Construct SSD1306 driver instance
    explicit constexpr SSD1306(bus::I2C &i2c_bus, Config const &config) noexcept :
        mixin::Configured<Config>{config}, _i2c_node{i2c_bus.createNode(config.i2c_node)} {}

    /// @brief Set display contrast level (0..255)
    [[nodiscard]] auto contrast(u8 value) noexcept -> internal::IicWriteResult {
        u8 const packet[]{CommandMode, Contrast, value};
        return _i2c_node.writePacket(packet);
    }

    /// @brief Enable or disable display power
    [[nodiscard]] auto power(bool on) noexcept -> internal::IicWriteResult {
        return sendCommand(on ? DisplayOn : DisplayOff);
    }

    /// @brief Invert display colors
    [[nodiscard]] auto invert(bool invert) noexcept -> internal::IicWriteResult {
        return sendCommand(invert ? InvertDisplay : NormalDisplay);
    }

    [[nodiscard]] static constexpr bool supportOrientation(Orientation orientation) noexcept {
        return orientation == Orientation::Normal or orientation == Orientation::MirrorX or orientation == Orientation::MirrorY;
    }

private:
    bus::I2C::Node _i2c_node;

    /// @brief Send single command to display
    [[nodiscard]] auto sendCommand(Command c) noexcept -> internal::IicWriteResult {
        u8 const packet[]{
            OneCommandMode,
            static_cast<u8>(c),
        };

        return _i2c_node.writeBuffer({packet});
    }

    KF_IMPL_DISPLAY_DRIVER(SSD1306, internal::SSD1306Image, internal::IicWriteResult);

    /// @brief Initialize display hardware via I2C
    auto initImpl() noexcept -> internal::IicWriteResult {
        static constexpr u8 init_commands[]{
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

        return _i2c_node.writeBuffer({init_commands});
    }

    constexpr void resetImpl() const noexcept {}

    auto sendImpl() noexcept -> internal::IicWriteResult {
        // Transfer software buffer to display via I2C

        static constexpr u8 set_area_commands[]{
            CommandMode,
            // Set full display window
            ColumnAddr,
            0,
            internal::SSD1306Image::comptime_width,
            PageAddr,
            0,
            PixelImpl::pages(internal::SSD1306Image::comptime_height) - 1,
        };

        KF_TRY(_i2c_node.writeBuffer({set_area_commands}));

        auto p = this->image().buffer().data();
        auto remaining = this->image().buffer().length();

        while (remaining > 0) {
            auto const chunk = math::min(this->config().chunk_size, remaining);

            KF_TRY(_i2c_node.writeMixed(Command::DataMode, {p, chunk}));

            p += chunk;
            remaining -= chunk;
        }

        return ok();
    }

    auto setOrientationImpl(Orientation orientation) noexcept -> internal::IicWriteResult {
        if (not supportOrientation(orientation)) {
            return ok();
        }

        constexpr auto flip_x = 0b01;
        constexpr auto flip_y = 0b10;
        auto const flags = static_cast<u8>(orientation);

        KF_TRY(sendCommand((flags & flip_x) ? FlipH : NormalH));
        KF_TRY(sendCommand((flags & flip_y) ? FlipV : NormalV));

        return ok();
    }
};

}// namespace kf::driver::display