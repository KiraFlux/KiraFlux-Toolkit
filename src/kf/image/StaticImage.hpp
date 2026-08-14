// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    image/StaticImage.hpp
/// @brief   Image with compile‑time dimensions (fixed buffer).

#pragma once

#include "kf/Array.hpp"
#include "kf/Slice.hpp"
#include "kf/core.hpp"
#include "kf/image/Image.hpp"
#include "kf/pixel/Pixel.hpp"
#include "kf/units.hpp"

namespace kf::image {

/// @brief Predefined bitmap image with compile-time dimensions
/// @tparam P Pixel format for the image
/// @tparam W Image width in pixels (compile-time constant)
/// @tparam H Image height in pixels (compile-time constant)
/// @note Represents a static image with fixed dimensions stored in memory.
/// The image buffer is embedded directly in the object and cannot be resized.
/// Useful for storing icons, logos, and other predefined graphics.
template<implements<pixel::PixelTag> P, units::Pixels W, units::Pixels H> struct StaticImage final : Image<StaticImage<P, W, H>, P> {

    using PixelImpl = P;
    using BufferType = typename PixelImpl::BufferType;
    using ColorType = typename PixelImpl::ColorType;

    using BufferStorage = Array<BufferType, PixelImpl::bufferSize(W, H)>;

    static constexpr usize comptime_width{W}, comptime_height{H};

    constexpr StaticImage() noexcept :
        _buffer{} {}

    explicit constexpr StaticImage(BufferStorage const &buffer) noexcept :
        _buffer{buffer} {}

private:
    /// @brief Raw image buffer data
    BufferStorage _buffer;

    KF_IMPL_IMAGE(StaticImage<P, W, H>, P);

    constexpr units::Pixels getWidthImpl() const noexcept {
        return W;
    }

    constexpr units::Pixels getHeightImpl() const noexcept {
        return H;
    }

    constexpr units::Pixels getStrideImpl() const noexcept {
        return getWidthImpl();
    }

    constexpr Slice<BufferType> getBufferImpl() noexcept {
        return _buffer.slice();
    }
};

}// namespace kf::image
