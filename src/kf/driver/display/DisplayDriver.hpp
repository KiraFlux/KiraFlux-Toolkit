// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/concepts.hpp"
#include "kf/image/Image.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"

#include "kf/driver/display/Orientation.hpp"

namespace kf::driver::display {

struct DisplayDriverTag {};

/// @brief CRTP base class for display driver implementations
/// @tparam Impl Concrete driver implementation type
/// @tparam ImageImpl Image buffer type
template<typename Impl, implements<image::ImageTag> ImageImpl, typename ResultType> struct DisplayDriver :

    DisplayDriverTag,
    meta::CRTP<Impl>,
    mixin::NonCopyable,
    mixin::Initable<Impl, ResultType()>,
    mixin::Resettable<Impl>

{

    /// @brief Mutable access at image buffer
    [[nodiscard]] ImageImpl &image() noexcept {
        return _screen_image;
    }

    /// @brief Readonly access at image buffer
    [[nodiscard]] const ImageImpl &image() const noexcept {
        return _screen_image;
    }

    /// @brief Transfer software buffer to display hardware
    [[nodiscard]] ResultType send() noexcept {
        return this->impl().sendImpl();
    }

    /// @brief Set display orientation.
    /// @param new_orientation New orientation value.
    [[nodiscard]] ResultType orientation(Orientation new_orientation) noexcept {
        return this->impl().setOrientationImpl(new_orientation);
    }

private:
    ImageImpl _screen_image{};///<  Software frame buffer for display operations
};

}// namespace kf::driver::display

#define KF_IMPL_DISPLAY_DRIVER(__impl__, __image_impl__, ...)                                  \
    friend struct ::kf::driver::display::DisplayDriver<__impl__, __image_impl__, __VA_ARGS__>; \
    KF_IMPL_INITABLE(__impl__, __VA_ARGS__());                                                 \
    KF_IMPL_RESETTABLE(__impl__)
