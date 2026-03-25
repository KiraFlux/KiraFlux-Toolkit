// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/image/Image.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"

#include "kf/drivers/display/Orientation.hpp"

namespace kf::drivers::display {
    
struct DisplayDriverTag {};

/// @brief CRTP base class for display driver implementations
/// @tparam DriverImpl Concrete driver implementation type
/// @tparam ImageImpl Image buffer type
template<typename DriverImpl, typename ImageImpl>
struct DisplayDriver : DisplayDriverTag,
                       mixin::Initable<DriverImpl, bool>,
                       mixin::NonCopyable,
                       mixin::Resettable<DriverImpl>,
                       meta::CRTP<DriverImpl> {
    KF_CHECK_IMPL(ImageImpl, image::ImageTag);

    /// @brief Mutable access at image buffer
    [[nodiscard]] ImageImpl &image() noexcept { return _screen_image; }

    /// @brief Transfer software buffer to display hardware
    /// @return true if success
    [[nodiscard]] bool send() noexcept { return this->impl().sendImpl(); }

    /// @brief Set display orientation.
    /// @param new_orientation New orientation value.
    /// @return true if success, false if orientation not supported.
    [[nodiscard]] bool orientation(Orientation new_orientation) noexcept { return this->impl().setOrientationImpl(new_orientation); }

private:
    ImageImpl _screen_image{};///<  Software frame buffer for display operations
};

}// namespace kf::drivers::display