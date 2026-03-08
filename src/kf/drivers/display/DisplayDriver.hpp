// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/image/Tag.hpp"
#include "kf/meta/type_check.hpp"

#include "kf/drivers/display/Orientation.hpp"
#include "kf/drivers/display/Tag.hpp"

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
    ImageImpl _screen_image{};

public:
    /// @brief image buffer
    [[nodiscard]] ImageImpl &image() noexcept { return _screen_image; }

    /// @brief Initialize the display hardware
    /// @return true if success
    [[nodiscard]] bool init() noexcept { return impl().initImpl(); }

    /// @brief Transfer software buffer to display hardware
    /// @return true if success
    [[nodiscard]] bool send() noexcept { return impl().sendImpl(); }

    /// @brief Set display orientation
    /// @return true if success
    [[nodiscard]] bool orientation(Orientation new_orientation) noexcept { return impl().setOrientationImpl(new_orientation); }

protected:
    [[nodiscard]] constexpr usize imageBufferSizeBytes() const noexcept {
        return sizeof(BufferType) * _screen_image.buffer().size();
    }

    // CRTP

private:
    [[nodiscard]] inline Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    [[nodiscard]] inline const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::drivers::display
