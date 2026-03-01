// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/image/Image.hpp"
#include "kf/image/StaticImage.hpp"
#include "kf/math/units.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/pixel/PixelTag.hpp"

namespace kf::image {

template<typename P, usize W, usize H> struct ViewportImage final : Image<ViewportImage<P, W, H>, P> {
    kf_crtp_check(P, pixel::PixelTag);

    using PixelImpl = P;
    using BufferType = typename PixelImpl::BufferType;
    using ColorType = typename PixelImpl::ColorType;

private:
    StaticImage<PixelImpl, W, H> _image{};///< Raw image buffer data
    Pixels _logical_width{W}, _logical_height{H};

public:
    void transposed(bool is_transposed) {
        _logical_width = is_transposed ? H : W;
        _logical_height = is_transposed ? W : H;
    }

    // CRTP
private:
    friend Image<ViewportImage<PixelImpl, W, H>, PixelImpl>;

    [[nodiscard]] constexpr Pixels getWidthImpl() const noexcept { return _logical_width; }

    [[nodiscard]] constexpr Pixels getHeightImpl() const noexcept { return _logical_height; }

    [[nodiscard]] constexpr Pixels getStrideImpl() const noexcept { return _logical_width; }

    [[nodiscard]] constexpr Slice<BufferType> getBufferImpl() noexcept {
        return _image.buffer().first(_logical_width * _logical_height);
    }

    [[nodiscard]] constexpr Slice<const BufferType> getBufferImpl() const noexcept {
        return _image.buffer().first(_logical_width * _logical_height);
    }
};

}// namespace kf::image