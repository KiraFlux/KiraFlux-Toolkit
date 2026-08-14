// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    image/ViewportImage.hpp
/// @brief   Image with optional transpose (swap width/height).

#pragma once

#include "kf/core.hpp"
#include "kf/image/Image.hpp"
#include "kf/image/StaticImage.hpp"
#include "kf/pixel/Pixel.hpp"
#include "kf/units.hpp"

namespace kf::image {

template<implements<pixel::PixelTag> P, usize W, usize H> struct ViewportImage final : Image<ViewportImage<P, W, H>, P> {

    using PixelImpl = P;
    using BufferType = typename PixelImpl::BufferType;
    using ColorType = typename PixelImpl::ColorType;

    /// @brief Set transpose status for image
    /// @note Swaps logical width <-> logical height
    void transposed(bool is_transposed) {
        _logical_width = is_transposed ? H : W;
        _logical_height = is_transposed ? W : H;
    }

    /// @brief Is image is actually transposed?
    [[nodiscard]] constexpr bool transposed() const noexcept {
        return W == _logical_width;
    }

private:
    StaticImage<PixelImpl, W, H> _image{};///< Raw image buffer data
    units::Pixels _logical_width{W}, _logical_height{H};

    KF_IMPL_IMAGE(ViewportImage<P, W, H>, P);

    [[nodiscard]] constexpr units::Pixels getWidthImpl() const noexcept {
        return _logical_width;
    }

    [[nodiscard]] constexpr units::Pixels getHeightImpl() const noexcept {
        return _logical_height;
    }

    [[nodiscard]] constexpr units::Pixels getStrideImpl() const noexcept {
        return getWidthImpl();
    }

    [[nodiscard]] constexpr Slice<BufferType> getBufferImpl() noexcept {
        return _image.buffer().first(_logical_width * _logical_height);
    }
};

}// namespace kf::image