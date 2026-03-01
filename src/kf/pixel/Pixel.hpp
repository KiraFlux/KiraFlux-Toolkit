// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/memory/Slice.hpp"

#include "kf/pixel/Tag.hpp"

namespace kf::pixel {

template<typename Impl, typename Tb, typename Tc, u8 bits> struct Pixel : Tag {
    // types

    using PositionType = i16;
    using BufferType = Tb;
    using ColorType = Tc;

    // values

    static constexpr auto bits_per_pixel{bits};

    template<PositionType W, PositionType H> static constexpr usize buffer_size{Impl::getBufferSizeImpl(W, H)};

    // conventions

    [[nodiscard]] static constexpr ColorType fromRgb(
        u8 r,
        u8 g,
        u8 b) noexcept { return Impl::fromRgbImpl(r, g, b); }

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
        Slice<const BufferType> src,
        PositionType src_w,
        PositionType src_h,
        Slice<BufferType> dst,
        PositionType dst_stride,
        PositionType dst_x,
        PositionType dst_y) noexcept {
        if (src_w <= 0 || src_h <= 0 || dst_stride <= 0) return;

        // Обрезка по целевому буферу
        const auto dst_total_h = dst.size() / dst_stride;
        if (dst_y >= dst_total_h) return;

        PositionType copy_w = src_w;
        PositionType copy_h = src_h;

        // Обрезка справа
        if (dst_x + copy_w > dst_stride) {
            if (dst_x >= dst_stride) return;
            copy_w = dst_stride - dst_x;
        }

        // Обрезка снизу
        if (dst_y + copy_h > dst_total_h) {
            copy_h = dst_total_h - dst_y;
        }

        if (copy_w <= 0 || copy_h <= 0) return;

        // Обрезка по размеру исходного буфера (на всякий случай)
        const usize src_pixels = src.size();
        if (static_cast<usize>(src_w) * src_h > src_pixels) {
            copy_h = kf::min(copy_h, static_cast<PositionType>(src_pixels / src_w));
            if (copy_h <= 0) return;
        }

        Impl::copyImpl(src, src_w, src_h, dst, dst_stride, dst_x, dst_y, copy_w, copy_h);
    }

    // CRTP

    friend Impl;

protected:
    using Base = Pixel;
};

}// namespace kf::pixel