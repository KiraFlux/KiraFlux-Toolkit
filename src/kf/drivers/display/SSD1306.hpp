// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Result.hpp"
#include "kf/algorithm.hpp"
#include "kf/bus/iic/IIC.hpp"
#include "kf/image/StaticImage.hpp"
#include "kf/pixel/MonochromePixel.hpp"
#include "kf/primitives.hpp"

#include "kf/drivers/display/DisplayDriver.hpp"
#include "kf/drivers/display/Orientation.hpp"

namespace kf::internal {

using SSD1306ImageImpl = image::StaticImage<pixel::MonochromePixel, 128, 64>;

}

namespace kf::drivers::display {

/// @brief SSD1306 OLED display driver for 128x64 monochrome panels
/// @tparam N Implementation of IIC bus Node
template<typename N> struct SSD1306 final :

    DisplayDriver<SSD1306<N>, internal::SSD1306ImageImpl, Result<void, typename N::Error>>

{
    KF_CHECK_IMPL(N, ::kf::bus::iic::IicNodeTag);

    using IicNodeImpl = N;
    using PixelImpl = typename internal::SSD1306ImageImpl::PixelImpl;
    using IicOperationResult = Result<void, typename IicNodeImpl::Error>;

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

    static constexpr u8 default_address = {0x3C};

    /// @brief Construct SSD1306 driver instance
    explicit SSD1306(IicNodeImpl &&node) noexcept : _node{std::move(node)} {}

    /// @brief Set display contrast level (0..255)
    [[nodiscard]] IicOperationResult contrast(u8 value) noexcept {
        const u8 packet[]{CommandMode, Contrast, value};
        return _node.writePacket(packet);
    }

    /// @brief Enable or disable display power
    [[nodiscard]] IicOperationResult power(bool on) noexcept {
        return sendCommand(on ? DisplayOn : DisplayOff);
    }

    /// @brief Invert display colors
    [[nodiscard]] IicOperationResult invert(bool invert) noexcept {
        return sendCommand(invert ? InvertDisplay : NormalDisplay);
    }

    [[nodiscard]] static constexpr bool supportOrientation(Orientation orientation) noexcept {
        return orientation == Orientation::Normal or orientation == Orientation::MirrorX or orientation == Orientation::MirrorY;
    }

private:
    IicNodeImpl _node;

    /// @brief Send single command to display
    [[nodiscard]] IicOperationResult sendCommand(Command c) noexcept {
        const u8 packet[]{OneCommandMode, static_cast<u8>(c)};
        return _node.writePacket(packet);
    }

    // impl
    using This = SSD1306<N>;

    KF_IMPL_INITABLE(This, IicOperationResult());
    /// @brief Initialize display hardware via I2C
    IicOperationResult initImpl() noexcept {
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

        return _node.writePacket(init_commands);
    }

    KF_IMPL_RESETTABLE(This);
    void resetImpl() const noexcept {}

    KF_IMPL(DisplayDriver<This, internal::SSD1306ImageImpl, IicOperationResult>);
    IicOperationResult sendImpl() noexcept {
        // Transfer software buffer to display via I2C

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

        KF_TRY(_node.writePacket(set_area_commands));

        auto p = this->image().buffer().data();
        auto remaining = this->image().buffer().length();

        while (remaining > 0) {
            const auto chunk = min(packet_size, remaining);

            KF_TRY(_node.writeMixed(Command::DataMode, {p, chunk}));

            p += chunk;
            remaining -= chunk;
        }

        return ok();
    }

    IicOperationResult setOrientationImpl(Orientation orientation) noexcept {
        if (not supportOrientation(orientation)) {
            return ok();
        }

        constexpr auto flip_x = 0b01;
        constexpr auto flip_y = 0b10;
        const auto flags = static_cast<u8>(orientation);

        KF_TRY(sendCommand((flags & flip_x) ? FlipH : NormalH));
        KF_TRY(sendCommand((flags & flip_y) ? FlipV : NormalV));

        return ok();
    }
};

}// namespace kf::drivers::display