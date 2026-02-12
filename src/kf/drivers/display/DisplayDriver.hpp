// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/attributes.hpp"
#include "kf/memory/Slice.hpp"

namespace kf {

/// @brief CRTP base class for display driver implementations
/// @tparam Impl Concrete driver implementation type
/// @tparam I Image buffer type
template<typename Impl, typename I> struct DisplayDriver {
    using Image = I;
    using ColorType = typename Image::ColorType;

protected:
    /// @brief Software frame buffer for display operations
    Image screen_image{};

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

    /// @brief image buffer
    kf_nodiscard Image &image() noexcept { return screen_image; }

    /// @brief Initialize the display hardware
    kf_nodiscard bool init() noexcept { return impl().initImpl(); }

    /// @brief Transfer software buffer to display hardware
    void send() const noexcept { c_impl().sendImpl(); }

    /// @brief Set display orientation
    void setOrientation(Orientation orientation) noexcept { impl().setOrientationImpl(orientation); }

protected:
    kf_nodiscard constexpr usize imageBufferSizeBytes() const noexcept {
        return sizeof(typename Image::BufferType) * screen_image.buffer().size();
    }

    // CRTP
private:
    friend Impl;
    inline Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    inline const Impl &c_impl() const noexcept { return *static_cast<const Impl *>(this); }

protected:
    using Base = DisplayDriver;
};

}// namespace kf
