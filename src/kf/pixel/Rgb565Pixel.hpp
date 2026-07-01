// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/pixel/Pixel.hpp"

namespace kf::pixel {

/// @brief 16-bit BIG ENDIAN RGB565 format (5-6-5 bits per channel)
struct Rgb565Pixel final : Pixel<Rgb565Pixel, u16, u16, 16> {
    // PixelFormat implementation
private:
    friend Base;

    static constexpr usize getBufferSizeImpl(usize width, usize height) noexcept {
        return width * height;
    }

    static constexpr ColorType fromRgbImpl(u8 r, u8 g, u8 b) noexcept {
        const auto color = (r >> 3) << 11 | ((g >> 2) << 5) | (b >> 3);
        // convert to BE
        return static_cast<ColorType>(((color & 0xFF) << 8) | (color >> 8));
    }

    static void setPixelImpl(Slice<BufferType> buffer, PositionType stride, PositionType abs_x, PositionType abs_y, ColorType color) noexcept {
        const auto target = buffer.data() + (abs_y * stride + abs_x);
        if (target < buffer.end()) {
            *target = color;
        }
    }

    static void fillImpl(

        Slice<BufferType> buffer,
        PositionType stride,
        PositionType offset_x, PositionType offset_y,
        PositionType width, PositionType height,
        ColorType color

        ) noexcept {
        const auto total_height = int(buffer.length()) / stride;
        const auto end_y = kf::min(offset_y + height, total_height);
        const auto end_x = kf::min(offset_x + width, int(stride));

        for (auto y = offset_y; y < end_y; y += 1) {
            for (auto x = offset_x; x < end_x; x += 1) {
                buffer[y * stride + x] = color;
            }
        }
    }

    static void copyImpl(

        Slice<const BufferType> src,
        PositionType src_w, PositionType src_h,

        Slice<BufferType> dst, PositionType dst_stride,
        PositionType dst_x, PositionType dst_y,

        PositionType copy_w, PositionType copy_h

        ) noexcept {
        for (auto y = 0; y < copy_h; y += 1) {
            const auto src_row = y * src_w;
            const auto dst_row = (dst_y + y) * dst_stride + dst_x;

            for (auto x = 0; x < copy_w; x += 1) {
                dst[dst_row + x] = src[src_row + x];
            }
        }
    }
};

}// namespace kf::pixel