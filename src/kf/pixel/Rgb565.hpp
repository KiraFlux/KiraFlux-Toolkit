// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/pixel/PixelFormat.hpp"


namespace kf {
namespace pixel {

/// @brief 16-bit BIG ENDIAN RGB565 format (5-6-5 bits per channel)
struct Rgb565 final : PixelFormat<Rgb565, u16, u16, 16> {
    // PixelFormat implementation
private:
    friend Base;

    static constexpr usize getBufferSizeImpl(u16 width, u16 height) noexcept {
        return width * height;
    }

    static constexpr ColorType fromRgbImpl(u8 r, u8 g, u8 b) noexcept {
        const auto color = (r >> 3) << 11 | ((g >> 2) << 5) | (b >> 3);
        // convert to BE
        return static_cast<ColorType>(((color & 0xFF) << 8) | (color >> 8));
    }

    static void setPixelImpl(BufferType *buffer, PositionType stride, PositionType abs_x, PositionType abs_y, ColorType color) noexcept {
        buffer[abs_y * stride + abs_x] = color;
    }

    static void fillImpl(BufferType *buffer, PositionType stride, PositionType offset_x, PositionType offset_y, PositionType width, PositionType height, ColorType color) noexcept {
        for (usize y = 0; y < height; y += 1) {
            const auto abs_y = offset_y + y;
            const usize row_start = abs_y * stride + offset_x;

            for (usize x = 0; x < width; x += 1) {
                buffer[row_start + x] = color;
            }
        }
    }

    static void copyImpl(
        const BufferType *source_buffer, PositionType source_width, PositionType source_height,
        BufferType *dest_buffer, PositionType dest_stride, PositionType dest_width, PositionType dest_height, PositionType dest_x, PositionType dest_y
    ) noexcept {

        // Boundary checks
        if (dest_x >= dest_width or dest_y >= dest_height) { return; }

        int copy_width = source_width;
        int copy_height = source_height;

        if (dest_x + copy_width > dest_width) {
            copy_width = dest_width - dest_x;
        }
        if (dest_y + copy_height > dest_height) {
            copy_height = dest_height - dest_y;
        }

        if (copy_width <= 0 or copy_height <= 0) { return; }

        for (usize y = 0; y < copy_height; y += 1) {
            const auto dest_row = dest_y + y;
            if (dest_row >= dest_height) { break; }

            const usize src_row_start = y * source_width;
            const usize dest_row_start = dest_row * dest_stride + dest_x;

            for (usize x = 0; x < copy_width; x += 1) {
                const auto dest_col = dest_x + x;
                if (dest_col >= dest_width) { break; }

                dest_buffer[dest_row_start + x] = source_buffer[src_row_start + x];
            }
        }
    }
};

}
}