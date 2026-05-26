// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Slice.hpp"
#include "kf/image/Image.hpp"
#include "kf/math/units.hpp"
#include "kf/memory/Array.hpp"
#include "kf/pixel/Pixel.hpp"

namespace kf::image {

/// @brief Predefined bitmap image with compile-time dimensions
/// @tparam P Pixel format for the image
/// @tparam W Image width in pixels (compile-time constant)
/// @tparam H Image height in pixels (compile-time constant)
/// @details Represents a static image with fixed dimensions stored in memory.
/// The image buffer is embedded directly in the object and cannot be resized.
/// Useful for storing icons, logos, and other predefined graphics.
template<typename P, math::Pixels W, math::Pixels H> struct StaticImage final : Image<StaticImage<P, W, H>, P> {
    KF_CHECK_IMPL(P, pixel::PixelTag);

    using PixelImpl = P;
    using BufferType = typename PixelImpl::BufferType;
    using ColorType = typename PixelImpl::ColorType;

    using BufferStorage = memory::Array<BufferType, PixelImpl::template buffer_size<W, H>>;

    explicit StaticImage(const BufferStorage &buffer) noexcept :
        _buffer{buffer} {}

    StaticImage() noexcept :
        _buffer{} {}

private:
    /// @brief Raw image buffer data
    /// @details Contains the pixel data for the entire image.
    BufferStorage _buffer;

    // impl
    using This = StaticImage<P, W, H>;

    KF_IMPL(Image<This, P>);

    [[nodiscard]] constexpr math::Pixels getWidthImpl() const noexcept { return W; }

    [[nodiscard]] constexpr math::Pixels getHeightImpl() const noexcept { return H; }

    [[nodiscard]] constexpr math::Pixels getStrideImpl() const noexcept { return getWidthImpl(); }

    [[nodiscard]] constexpr Slice<BufferType> getBufferImpl() noexcept {
        return {_buffer.data(), _buffer.size()};
    }

    [[nodiscard]] constexpr Slice<const BufferType> getBufferImpl() const noexcept {
        return {_buffer.data(), _buffer.size()};
    }
};

}// namespace kf::image
