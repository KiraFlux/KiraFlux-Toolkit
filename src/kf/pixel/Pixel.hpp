// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Slice.hpp"
#include "kf/algorithm.hpp"
#include "kf/primitives.hpp"

namespace kf::pixel {

struct PixelTag {};

template<typename Impl, typename Tb, typename Tc, u8 bits> struct Pixel : PixelTag {
    // types

    using PositionType = i16;
    using BufferType = Tb;
    using ColorType = Tc;

    // values

    static constexpr auto bits_per_pixel{bits};

    [[nodiscard]] static constexpr usize bufferSize(usize width, usize height) noexcept {
        return Impl::getBufferSizeImpl(width, height);
    }

    // conventions

    [[nodiscard]] static constexpr ColorType fromRgb(u8 r, u8 g, u8 b) noexcept {
        return Impl::fromRgbImpl(r, g, b);
    }

    // draw

    /// @brief set pixel color
    static void setPixel(
        Slice<BufferType> buffer,
        PositionType stride,
        PositionType abs_x,
        PositionType abs_y,
        ColorType color) noexcept {
        Impl::setPixelImpl(
            buffer, stride,
            abs_x, abs_y,
            color);
    }

    /// @brief Effective fill rectangular region with specified color
    static void fill(
        Slice<BufferType> buffer,
        PositionType stride,
        PositionType offset_x,
        PositionType offset_y,
        PositionType width,
        PositionType height,
        ColorType color) noexcept {
        Impl::fillImpl(
            buffer, stride,
            offset_x, offset_y,
            width, height,
            color);
    }

    /// @brief Copy rectangular region from source to destination buffer
    static void copy(
        Slice<const BufferType> source_buffer,
        PositionType source_width,
        PositionType source_height,
        Slice<BufferType> dest_buffer,
        PositionType dest_stride,
        PositionType dest_x,
        PositionType dest_y) noexcept {
        if (source_width <= 0 or source_height <= 0 or dest_stride <= 0) { return; }

        const auto dst_total_h = dest_buffer.size() / dest_stride;
        if (dest_y >= dst_total_h) { return; }

        auto copy_width = source_width;
        auto copy_height = source_height;

        if (dest_x + copy_width > dest_stride) {
            if (dest_x >= dest_stride) { return; }
            copy_width = dest_stride - dest_x;
        }

        if (dest_y + copy_height > dst_total_h) {
            copy_height = dst_total_h - dest_y;
        }

        if (copy_width <= 0 or copy_height <= 0) { return; }

        const usize src_pixels = source_buffer.size();
        if (static_cast<usize>(source_width) * source_height > src_pixels) {
            copy_height = kf::min(copy_height, static_cast<PositionType>(src_pixels / source_width));
            if (copy_height <= 0) { return; }
        }

        Impl::copyImpl(source_buffer, source_width, source_height, dest_buffer, dest_stride, dest_x, dest_y, copy_width, copy_height);
    }

    // CRTP

    friend Impl;

protected:
    using Base = Pixel;
};

}// namespace kf::pixel