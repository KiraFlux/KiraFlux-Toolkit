// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/core/attributes.hpp>
#include <kf/core/pixel_traits.hpp>
#include <kf/memory/Slice.hpp>


namespace kf {

/// @brief CRTP base class for display driver implementations
/// @tparam Impl Concrete driver implementation type
/// @tparam F Physical display pixel format
/// @tparam W Physical display width in pixels
/// @tparam H Physical display height in pixels
template<typename Impl, PixelFormat F, usize W, usize H> struct DisplayDriver {
    friend Impl;

protected:
    using Base = DisplayDriver;
    using traits = pixel_traits<F>;

public:
    /// @brief Type used for buffer storage
    using BufferType = typename traits::BufferType;

    /// @brief Type used for color representation
    using ColorType = typename traits::ColorType;

    /// @brief Pixel format
    static constexpr auto pixel_format{F};

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
    static constexpr auto buffer_items{traits::template buffer_size<W, H>};

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
    kf_nodiscard bool init() { return impl().initImpl(); }

    /// @brief Get current display width in pixels (may differ from physical width due to orientation)
    kf_nodiscard u8 width() const { return c_impl().getWidthImpl(); }

    /// @brief Get current display height in pixels (may differ from physical width due to orientation)
    kf_nodiscard u8 height() const { return c_impl().getHeightImpl(); }

    /// @brief Transfer software buffer to display hardware
    void send() const { c_impl().sendImpl(); }

    /// @brief Set display orientation
    void setOrientation(Orientation orientation) { impl().setOrientationImpl(orientation); }

    /// @brief Get writable software frame buffer
    kf_nodiscard Slice<BufferType> buffer() { return {software_screen_buffer, buffer_items}; }

    /// @brief Get maximum valid X coordinate for current orientation
    kf_nodiscard u8 maxX() const { return width() - 1; }

    /// @brief Get maximum valid Y coordinate for current orientation
    kf_nodiscard u8 maxY() const { return height() - 1; }

private:
    inline Impl &impl() { return *static_cast<Impl *>(this); }

    inline const Impl &c_impl() const { return *static_cast<const Impl *>(this); }
};

}// namespace kf
