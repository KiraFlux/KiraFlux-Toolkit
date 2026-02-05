// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/pixel/PixelFormat.hpp"

namespace kf {
namespace pixel {

/// @brief 16-bit BIG ENDIAN RGB565 format (5-6-5 bits per channel)
struct Rgb565 final : PixelFormat<Rgb565, u16, u16, 16> {
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

    static void fillImpl(Slice<BufferType> buffer, PositionType stride, PositionType offset_x, PositionType offset_y, PositionType width, PositionType height, ColorType color) noexcept {
        const PositionType total_height = buffer.size() / stride;

        const auto end_y = kf::min(offset_y + height, int(total_height));
        const auto end_x = kf::min(offset_x + width, int(stride));

        for (auto y = offset_y; y < end_y; y += 1) {
            for (auto x = offset_x; x < end_x; x += 1) {
                buffer[y * stride + x] = color;
            }
        }
    }

    static void copyImpl(
        const Slice<BufferType> source,
        PositionType src_width,
        PositionType src_height,
        Slice<BufferType> dest,
        PositionType dst_stride,
        PositionType dst_width,
        PositionType dst_height,
        PositionType dst_x,
        PositionType dst_y) noexcept {

        if (src_width <= 0 or src_height <= 0 or
            dst_stride <= 0 or dst_width <= 0 or dst_height <= 0) {
            return;
        }

        if (static_cast<usize>(src_width) * src_height > source.size() or
            static_cast<usize>(dst_stride) * dst_height > dest.size()) {
            return;
        }

        const auto copy_w = kf::min(int(src_width), dst_width - dst_x);
        const auto copy_h = kf::min(int(src_height), dst_height - dst_y);

        if (copy_w <= 0 or copy_h <= 0) {
            return;
        }

        for (PositionType y = 0; y < copy_h; ++y) {
            const PositionType src_y = y;
            const PositionType dst_y_abs = dst_y + y;

            if (dst_y_abs >= dst_height) {
                break;
            }

            const usize src_row_start = static_cast<usize>(src_y) * src_width;
            const usize dst_row_start = static_cast<usize>(dst_y_abs) * dst_stride + dst_x;

            // Копируем строку
            for (PositionType x = 0; x < copy_w; ++x) {
                dest[dst_row_start + x] = source[src_row_start + x];
            }
        }
    }
};

}// namespace pixel
}// namespace kf