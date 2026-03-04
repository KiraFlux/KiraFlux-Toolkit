// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/drivers/display/Tag.hpp"
#include "kf/image/Tag.hpp"
#include "kf/memory/Slice.hpp"
#include "kf/meta/type_check.hpp"

namespace kf::drivers::display {

/// @brief CRTP base class for display driver implementations
/// @tparam Impl Concrete driver implementation type
/// @tparam I Image buffer type
template<typename Impl, typename I> struct DisplayDriver : drivers::display::Tag {
    kf_crtp_check(I, image::Tag);

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
    /// @return true if success
    [[nodiscard]] bool init() noexcept { return impl().initImpl(); }

    /// @brief Transfer software buffer to display hardware
    /// @return true if success
    [[nodiscard]] bool send() const noexcept { return c_impl().sendImpl(); }

    /// @brief Set display orientation
    /// @return true if success
    [[nodiscard]] bool orientation(Orientation new_orientation) noexcept { return impl().setOrientationImpl(new_orientation); }

protected:
    [[nodiscard]] constexpr usize imageBufferSizeBytes() const noexcept {
        return sizeof(BufferType) * screen_image.buffer().size();
    }

    // CRTP
protected:
    friend Impl;
    using Base = DisplayDriver;

private:
    [[nodiscard]] inline Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    [[nodiscard]] inline const Impl &c_impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf
