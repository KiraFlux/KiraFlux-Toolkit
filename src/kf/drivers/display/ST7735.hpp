// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "kf/core/aliases.hpp"
#include "kf/core/pixel_traits.hpp"
#include "kf/drivers/display/DisplayDriver.hpp"


namespace kf {

/// @brief ST7735 TFT display driver for 128x160 RGB565 panels
struct ST7735 : DisplayDriver<ST7735, PixelFormat::RGB565, 128, 160> {
    friend Base;

private:
    /// @brief Memory Access Control (MADCTL) register bits
    enum MadCtl : u8 {
        RgbMode = 0x00,///< RGB color order
        BgrMode = 0x08,///< BGR color order

        MirrorTranspose = 0x20,///< Swap X and Y axes (rotation)
        MirrorX = 0x40,        ///< Horizontal mirror
        MirrorY = 0x80,        ///< Vertical mirror
    };

public:
    /// @brief Hardware configuration settings for ST7735
    struct Config {
        u32 spi_frequency;      ///< SPI clock frequency in Hz
        Orientation orientation;///< Initial display orientation
        u8 pin_spi_slave_select;///< SPI chip select pin
        u8 pin_data_command;    ///< Data/command selection pin
        u8 pin_reset;           ///< Reset pin

        /// @brief Construct ST7735 hardware settings
        /// @param spi_cs SPI chip select GPIO
        /// @param dc Data/command selection GPIO
        /// @param reset Reset pin GPIO
        /// @param spi_freq SPI frequency (default 27MHz)
        /// @param orientation Initial orientation (default Normal)
        constexpr explicit Config(
            gpio_num_t spi_cs,
            gpio_num_t dc,
            gpio_num_t reset,
            u32 spi_freq = 27000000u,
            Orientation orientation = Orientation::Normal
        ) :
            pin_spi_slave_select{static_cast<u8>(spi_cs)},
            pin_data_command{static_cast<u8>(dc)},
            pin_reset{static_cast<u8>(reset)},
            spi_frequency{spi_freq},
            orientation{orientation} {}
    };

private:
    const Config &settings;///< Hardware configuration
    SPIClass &spi;           ///< SPI bus instance

    u8 logical_width{phys_width};        ///< Current logical width (after orientation)
    u8 logical_height{phys_height};      ///< Current logical height (after orientation)
    u8 madctl_base_mode{MadCtl::BgrMode};///< Base MADCTL value (BGR color order)

public:
    /// @brief Construct ST7735 driver instance
    /// @param settings Hardware configuration
    /// @param spi_instance SPI bus instance to use
    explicit ST7735(const Config &settings, SPIClass &spi_instance) :
        settings{settings}, spi{spi_instance} {}

private:
    // DisplayDriver interface implementation

    /// @brief Get current logical display width (after orientation transform)
    kf_nodiscard u8 getWidthImpl() const { return logical_width; }

    /// @brief Get current logical display height (after orientation transform)
    kf_nodiscard u8 getHeightImpl() const { return logical_height; }

    /// @brief Initialize display hardware via SPI
    /// @return Always returns true (hardware errors not checked)
    kf_nodiscard bool initImpl() {
        pinMode(settings.pin_spi_slave_select, OUTPUT);
        pinMode(settings.pin_data_command, OUTPUT);
        pinMode(settings.pin_reset, OUTPUT);

        spi.begin();
        spi.setFrequency(settings.spi_frequency);

        digitalWrite(settings.pin_reset, LOW);
        delay(10);
        digitalWrite(settings.pin_reset, HIGH);
        delay(120);

        sendCommand(Command::SWRESET);
        delay(150);

        sendCommand(Command::SLPOUT);
        delay(255);

        sendCommand(Command::COLMOD);
        const u8 color_mode{0x05};// 16-bit color (RGB565)
        sendData(&color_mode, sizeof(color_mode));

        setOrientation(settings.orientation);

        sendCommand(Command::DISPON);
        delay(100);

        return true;
    }

    /// @brief Transfer software buffer to display via SPI in one operation
    void sendImpl() const {
        sendCommand(Command::RAMWR);
        sendData(reinterpret_cast<const u8 *>(software_screen_buffer),
                 sizeof(software_screen_buffer));
    }

    /// @brief Apply orientation transformation with full 6-way support
    /// @param orientation Display orientation mode
    void setOrientationImpl(Orientation orientation) {
        constexpr u8 orient_to_transform[]{
            0,                                        // Orientation::Normal
            MadCtl::MirrorX,                          // Orientation::MirrorX
            MadCtl::MirrorY,                          // Orientation::MirrorY
            MadCtl::MirrorX | MadCtl::MirrorY,        // Orientation::Flip
            MadCtl::MirrorX | MadCtl::MirrorTranspose,// Orientation::ClockWise
            MadCtl::MirrorY | MadCtl::MirrorTranspose,// Orientation::CounterClockWise
        };

        const u8 madctl = madctl_base_mode | orient_to_transform[static_cast<u8>(orientation)];

        if (madctl & MadCtl::MirrorTranspose) {
            logical_width = phys_height;
            logical_height = phys_width;
        } else {
            logical_width = phys_width;
            logical_height = phys_height;
        }

        sendCommand(Command::MADCTL);
        sendData(&madctl, sizeof(madctl));

        u8 data[4] = {0x00, 0x00, 0x00, static_cast<u8>(logical_width - 1)};
        sendCommand(Command::CASET);
        sendData(data, sizeof(data));

        data[3] = logical_height - 1;
        sendCommand(Command::RASET);
        sendData(data, sizeof(data));
    }

    // Low-level SPI communication

    /// @brief Send data bytes to display
    /// @param data Pointer to data buffer
    /// @param size Number of bytes to send
    void sendData(const u8 *data, usize size) const {
        digitalWrite(settings.pin_data_command, HIGH);
        digitalWrite(settings.pin_spi_slave_select, LOW);
        spi.writeBytes(data, size);
        digitalWrite(settings.pin_spi_slave_select, HIGH);
    }

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

    /// @brief Send single command to display
    /// @param command ST7735 command byte
    void sendCommand(Command command) const {
        digitalWrite(settings.pin_data_command, LOW);
        digitalWrite(settings.pin_spi_slave_select, LOW);
        spi.write(static_cast<u8>(command));
        digitalWrite(settings.pin_spi_slave_select, HIGH);
    }
};

}// namespace kf
