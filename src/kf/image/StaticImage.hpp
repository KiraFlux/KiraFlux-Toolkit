// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/image/Image.hpp"
#include "kf/math/units.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/Slice.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/pixel/Tag.hpp"

namespace kf::image {

/// @brief Predefined bitmap image with compile-time dimensions
/// @tparam P Pixel format for the image
/// @tparam W Image width in pixels (compile-time constant)
/// @tparam H Image height in pixels (compile-time constant)
/// @details Represents a static image with fixed dimensions stored in memory.
/// The image buffer is embedded directly in the object and cannot be resized.
/// Useful for storing icons, logos, and other predefined graphics.
template<typename P, math::Pixels W, math::Pixels H> struct StaticImage final : Image<StaticImage<P, W, H>, P> {
    kf_crtp_check(P, pixel::Tag);
    
    using PixelImpl = P;
    using BufferType = typename PixelImpl::BufferType;
    using ColorType = typename PixelImpl::ColorType;

    using BufferStorage = memory::Array<BufferType, PixelImpl::template buffer_size<W, H>>;

private:
    /// @brief Raw image buffer data
    /// @details Contains the pixel data for the entire image.
    BufferStorage _buffer;

public:
    explicit StaticImage(const BufferStorage &buffer) noexcept :
        _buffer{buffer} {}

    StaticImage() noexcept :
        _buffer{} {}

    // CRTP
private:
    friend Image<StaticImage<PixelImpl, W, H>, PixelImpl>;

    [[nodiscard]] constexpr math::Pixels getWidthImpl() const noexcept { return W; }

    [[nodiscard]] constexpr math::Pixels getHeightImpl() const noexcept { return H; }

    [[nodiscard]] constexpr math::Pixels getStrideImpl() const noexcept { return getWidthImpl(); }

    [[nodiscard]] constexpr memory::Slice<BufferType> getBufferImpl() noexcept {
        return {_buffer.data(), _buffer.size()};
    }

    [[nodiscard]] constexpr memory::Slice<const BufferType> getBufferImpl() const noexcept {
        return {_buffer.data(), _buffer.size()};
    }
};

}// namespace kf::image
