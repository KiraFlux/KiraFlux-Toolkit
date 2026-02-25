// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/image/Image.hpp"
#include "kf/math/units.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/Slice.hpp"

namespace kf::image {

/// @brief Predefined bitmap image with compile-time dimensions
/// @tparam F Pixel format for the image
/// @tparam W Image width in pixels (compile-time constant)
/// @tparam H Image height in pixels (compile-time constant)
/// @details Represents a static image with fixed dimensions stored in memory.
/// The image buffer is embedded directly in the object and cannot be resized.
/// Useful for storing icons, logos, and other predefined graphics.
template<typename F, Pixels W, Pixels H> struct StaticImage final : Image<StaticImage<F, W, H>, F> {
    using PixelImpl = F;
    using BufferType = typename F::BufferType;
    using ColorType = typename F::ColorType;

    using BufferStorage = Array<BufferType, F::template buffer_size<W, H>>;

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
    friend Image<StaticImage<F, W, H>, F>;

    [[nodiscard]] constexpr Pixels getWidthImpl() const noexcept { return W; }

    [[nodiscard]] constexpr Pixels getHeightImpl() const noexcept { return H; }

    [[nodiscard]] constexpr Pixels getStrideImpl() const noexcept { return getWidthImpl(); }

    [[nodiscard]] constexpr Slice<BufferType> getBufferImpl() noexcept {
        return {_buffer.data(), _buffer.size()};
    }

    [[nodiscard]] constexpr Slice<const BufferType> getBufferImpl() const noexcept {
        return {_buffer.data(), _buffer.size()};
    }
};

}// namespace kf::image
