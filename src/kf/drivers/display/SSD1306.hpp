// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Wire.h>

#include "kf/core/aliases.hpp"
#include "kf/core/pixel_traits.hpp"
#include "kf/drivers/display/DisplayDriver.hpp"

namespace kf {

/// @brief SSD1306 OLED display driver for 128x64 monochrome panels
struct SSD1306 : DisplayDriver<SSD1306, PixelFormat::Monochrome, 128, 64> {
    friend Base;

private:
    /// @brief I2C device address (default 0x3C)
    const u8 address;

public:
    /// @brief Construct SSD1306 driver instance
    /// @param address I2C address (typically 0x3C or 0x3D)
    explicit SSD1306(u8 address = 0x3C) :
        address{address} {}

    /// @brief Set display contrast level
    /// @param value Contrast value (0-255)
    void setContrast(u8 value) const {
        Wire.beginTransmission(address);
        (void) Wire.write(CommandMode);
        (void) Wire.write(Contrast);
        (void) Wire.write(value);
        (void) Wire.endTransmission();
    }

    /// @brief Enable or disable display power
    /// @param on true to power on, false to power off
    void setPower(bool on) {
        sendCommand(on ? DisplayOn : DisplayOff);
    }

    /// @brief Invert display colors
    /// @param invert true for inverted (white on black), false for normal
    void invert(bool invert) {
        sendCommand(invert ? InvertDisplay : NormalDisplay);
    }

private:
    // DisplayDriver interface implementation

    /// @brief Get physical display width
    kf_nodiscard static u8 getWidthImpl() { return phys_width; }

    /// @brief Get physical display height
    kf_nodiscard static u8 getHeightImpl() { return phys_height; }

    /// @brief Initialize display hardware via I2C
    /// @return true if initialization successful
    kf_nodiscard bool initImpl() const {
        static constexpr u8 init_commands[] = {
            CommandMode,

            // Turn off for safe configuration
            DisplayOff,

            // Clock divider
            ClockDiv, 0x80,

            // Enable internal charge pump
            ChargePump, 0x14,

            // Horizontal addressing mode
            AddressingMode, Horizontal,

            // Default contrast 127
            Contrast, 0x7F,

            // VCOM voltage
            SetVcomDetect, 0x40,

            // Normal orientation
            NormalH, NormalV,

            // Turn display on
            DisplayOn,

            // Pin configuration (128x64)
            SetComPins, 0x12,

            // Multiplex (64 lines)
            SetMultiplex, 0x3F};

        if (not Wire.begin()) { return false; }

        Wire.beginTransmission(address);

        const auto written = Wire.write(init_commands, sizeof(init_commands));
        if (sizeof(init_commands) != written) { return false; }

        const u8 end_transmission_code = Wire.endTransmission();
        return 0 == end_transmission_code;
    }

    /// @brief Transfer software buffer to display via I2C in 64-byte packets
    void sendImpl() const {
        static constexpr auto packet_size = 64;// Optimal for ESP32 performance

        static constexpr u8 set_area_commands[] = {
            CommandMode,
            // Set full display window
            ColumnAddr,
            0,
            max_phys_x,
            PageAddr,
            0,
            Traits::template pages<phys_height> - 1,
        };

        Wire.beginTransmission(address);
        (void) Wire.write(set_area_commands, sizeof(set_area_commands));
        (void) Wire.endTransmission();

        auto p = software_screen_buffer;
        const auto *end = p + sizeof(software_screen_buffer);

        while (p < end) {
            Wire.beginTransmission(address);
            (void) Wire.write(Command::DataMode);
            (void) Wire.write(p, packet_size);
            (void) Wire.endTransmission();

            p += packet_size;
        }
    }

    /// @brief Apply orientation transformation (only flip operations supported)
    /// @param orientation Display orientation (Normal, MirrorX, MirrorY, or Flip)
    void setOrientationImpl(Orientation orientation) {
        constexpr auto flip_x = 0b01;
        constexpr auto flip_y = 0b10;

        const u8 flags = static_cast<u8>(orientation) & (flip_x | flip_y);
        sendCommand((flags & flip_x) ? FlipH : NormalH);
        sendCommand((flags & flip_y) ? FlipV : NormalV);
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
    /// @param command SSD1306 command byte
    void sendCommand(Command command) const {
        Wire.beginTransmission(address);
        (void) Wire.write(OneCommandMode);
        (void) Wire.write(static_cast<u8>(command));
        (void) Wire.endTransmission();
    }
};

}// namespace kf
