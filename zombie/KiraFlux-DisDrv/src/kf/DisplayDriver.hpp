// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/attributes.hpp>
#include <kf/pixel_traits.hpp>
#include <kf/slice.hpp>

namespace kf {

/// @brief CRTP base class for display driver implementations
/// @tparam Impl Concrete driver implementation type
/// @tparam Format Physical display pixel format
/// @tparam W Physical display width in pixels
/// @tparam H Physical display height in pixels
template<typename Impl, PixelFormat Format, usize W, usize H> struct DisplayDriver {
    friend Impl;

protected:
    using Base = DisplayDriver;
    using Traits = PixelTraits<Format>;

public:
    /// @brief Type used for buffer storage
    using BufferType = typename Traits::BufferType;

    /// @brief Type used for color representation
    using ColorType = typename Traits::ColorType;

protected:
    /// @brief Physical display width
    static constexpr auto phys_width{W};

    /// @brief Maximum physical X coordinate
    static constexpr auto max_phys_x{phys_width - 1};

    /// @brief Physical display height
    static constexpr auto phys_height{H};

    /// @brief Maximum physical Y coordinate
    static constexpr auto max_phys_y{phys_height - 1};

    /// @brief Required buffer size for the display
    static constexpr auto buffer_items{Traits::template buffer_size<W, H>};

    /// @brief Software frame buffer for display operations
    BufferType software_screen_buffer[buffer_items]{};

public:
    /// @brief Display orientation modes
    enum class Orientation : u8 {
        Normal = 0,          ///< Default orientation
        MirrorX = 1,         ///< Horizontal mirror
        MirrorY = 2,         ///< Vertical mirror
        Flip = 3,            ///< 180-degree rotation
        ClockWise = 4,       ///< 90-degree clockwise rotation
        CounterClockWise = 5,///< 90-degree counterclockwise rotation
    };

    /// @brief Initialize the display hardware
    /// @return true if initialization successful
    kf_nodiscard bool init() { return impl().initImpl(); }

    /// @brief Get current display width in pixels
    /// @return Display width (may differ from physical width due to orientation)
    kf_nodiscard u8 width() const { return c_impl().getWidthImpl(); }

    /// @brief Get current display height in pixels
    /// @return Display height (may differ from physical height due to orientation)
    kf_nodiscard u8 height() const { return c_impl().getHeightImpl(); }

    /// @brief Transfer software buffer to display hardware
    void send() const { c_impl().sendImpl(); }

    /// @brief Set display orientation
    /// @param orientation New orientation mode
    void setOrientation(Orientation orientation) { impl().setOrientationImpl(orientation); }

    /// @brief Get writable software frame buffer
    /// @return Slice providing access to frame buffer memory
    kf_nodiscard slice<BufferType> buffer() { return {software_screen_buffer, buffer_items}; }

    /// @brief Get maximum valid X coordinate for current orientation
    /// @return Maximum X coordinate
    kf_nodiscard u8 maxX() const { return width() - 1; }

    /// @brief Get maximum valid Y coordinate for current orientation
    /// @return Maximum Y coordinate
    kf_nodiscard u8 maxY() const { return height() - 1; }

private:
    inline Impl &impl() { return *static_cast<Impl *>(this); }
    inline const Impl &c_impl() const { return *static_cast<const Impl *>(this); }
};

}// namespace kf
