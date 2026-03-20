// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/image/Image.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"

#include "kf/drivers/display/Orientation.hpp"

namespace kf::drivers::display {

struct DisplayDriverTag {};

/// @brief CRTP base class for display driver implementations
/// @tparam Impl Concrete driver implementation type
/// @tparam I Image buffer type
template<typename Impl, typename I>
struct DisplayDriver : DisplayDriverTag,
                       mixin::Initable<Impl, bool>,
                       mixin::NonCopyable,
                       mixin::Resettable<Impl>,
                       meta::CRTP<Impl> {
    kf_crtp_check(I, image::ImageTag);

    using ImageImpl = I;
    using ColorType = typename I::ColorType;
    using BufferType = typename I::BufferType;

    /// @brief image buffer
    [[nodiscard]] ImageImpl &image() noexcept { return _screen_image; }

    /// @brief Transfer software buffer to display hardware
    /// @return true if success
    [[nodiscard]] bool send() noexcept { return this->impl().sendImpl(); }

    /// @brief Set display orientation.
    /// @param new_orientation New orientation value.
    /// @return true if success, false if orientation not supported.
    [[nodiscard]] bool orientation(Orientation new_orientation) noexcept { return this->impl().setOrientationImpl(new_orientation); }

private:
    /// @brief Software frame buffer for display operations
    ImageImpl _screen_image{};
};

}// namespace kf::drivers::display
