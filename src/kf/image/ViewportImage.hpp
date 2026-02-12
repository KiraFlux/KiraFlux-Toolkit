// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/image/Image.hpp"
#include "kf/image/StaticImage.hpp"
#include "kf/math/units.hpp"

namespace kf::image {

template<typename F, usize W, usize H> struct ViewportImage final : Image<ViewportImage<F, W, H>, F> {
    using PixelImpl = F;
    using BufferType = typename F::BufferType;
    using ColorType = typename F::ColorType;

private:
    /// @brief Raw image buffer data
    StaticImage<F, W, H> image{};
    Pixels logical_width{W}, logical_height{H};

public:
    void setTransposed(bool transposed) {
        logical_width = transposed ? H : W;
        logical_height = transposed ? W : H;
    }

    // CRTP
private:
    friend Image<ViewportImage<F, W, H>, F>;

    kf_nodiscard constexpr Pixels getWidthImpl() const noexcept { return logical_width; }

    kf_nodiscard constexpr Pixels getHeightImpl() const noexcept { return logical_height; }

    kf_nodiscard constexpr Pixels getStrideImpl() const noexcept { return logical_width; }

    kf_nodiscard constexpr Slice<BufferType> getBufferImpl() noexcept {
        return image.buffer().first(logical_width * logical_height);
    }

    kf_nodiscard constexpr Slice<const BufferType> getBufferImpl() const noexcept {
        return image.buffer().first(logical_width * logical_height);
    }
};

}// namespace kf::image