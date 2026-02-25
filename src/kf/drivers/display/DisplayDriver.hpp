// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/Slice.hpp"

namespace kf {

/// @brief CRTP base class for display driver implementations
/// @tparam Impl Concrete driver implementation type
/// @tparam I Image buffer type
template<typename Impl, typename I> struct DisplayDriver {
    using ImageImpl = I;
    using ColorType = typename I::ColorType;
    using BufferType = typename I::BufferType;

protected:
    /// @brief Software frame buffer for display operations
    ImageImpl screen_image{};

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
    [[nodiscard]] ImageImpl &image() noexcept { return screen_image; }

    /// @brief Initialize the display hardware
    [[nodiscard]] bool init() noexcept { return static_cast<Impl *>(this)->initImpl(); }

    /// @brief Transfer software buffer to display hardware
    void send() const noexcept { static_cast<const Impl *>(this)->sendImpl(); }

    /// @brief Set display orientation
    void setOrientation(Orientation orientation) noexcept { static_cast<Impl *>(this)->setOrientationImpl(orientation); }

protected:
    [[nodiscard]] constexpr usize imageBufferSizeBytes() const noexcept {
        return sizeof(BufferType) * screen_image.buffer().size();
    }

    // CRTP
protected:
    friend Impl;
    using Base = DisplayDriver;
};

}// namespace kf
