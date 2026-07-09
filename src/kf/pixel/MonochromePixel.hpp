// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Slice.hpp"
#include "kf/algorithm.hpp"
#include "kf/pixel/Pixel.hpp"

namespace kf::pixel {

/// @brief 1-bit monochrome format (1 bit per pixel)
struct MonochromePixel final : Pixel<MonochromePixel, u8, bool, 1> {

    static constexpr u8 page_height{8};///< Vertical pixels per memory page

    /// @brief Calculate number of memory pages for given height
    /// @return Number of 8-pixel memory pages
    template<usize H> static constexpr usize pages = (H + 7) / 8;

    // PixelFormat implementation
private:
    friend Base;

    static constexpr usize getBufferSizeImpl(usize width, usize height) noexcept {
        return (width * height + 7) / 8;
    }

    static constexpr ColorType fromRgbImpl(u8 r, u8 b, u8 g) noexcept {
        return (r + g + b) > 128 * 3;
    }

    static void setPixelImpl(Slice<BufferType> buffer, PositionType stride, PositionType abs_x, PositionType abs_y, ColorType color) noexcept {
        const auto page = abs_y / page_height;
        const auto bit_mask = static_cast<u8>(1 << (abs_y % page_height));
        const usize index = page * stride + abs_x;

        if (color) {
            buffer[index] |= bit_mask;
        } else {
            buffer[index] &= ~bit_mask;
        }
    }

    static void fillImpl(
        Slice<BufferType> buffer,
        PositionType stride,
        PositionType offset_x,
        PositionType offset_y,
        PositionType width,
        PositionType height,
        ColorType on) noexcept {
        const auto start_page = offset_y / page_height;
        const auto end_page = ((offset_y + height + page_height - 1) / page_height);
        const u8 fill_byte = on ? 0xFF : 0x00;

        for (auto page = start_page; page < end_page; page += 1) {
            const u8 mask = calculatePageMask(page, offset_y, height);
            if (mask == 0) { continue; }

            for (auto x = 0; x < width; x += 1) {
                const auto abs_x = offset_x + x;
                if (abs_x < 0 or abs_x >= stride) { continue; }

                const usize index = page * stride + abs_x;
                if (index < buffer.length()) {
                    buffer[index] = (buffer[index] & ~mask) | (fill_byte & mask);
                }
            }
        }
    }

    static void copyImpl(
        Slice<const BufferType> src,
        PositionType src_w,
        PositionType src_h,
        PositionType src_x,
        PositionType src_y,
        PositionType copy_w,
        PositionType copy_h,
        Slice<BufferType> dst,
        PositionType dst_stride,
        PositionType dst_x,
        PositionType dst_y) noexcept {
        const PositionType src_page_h = page_height;
        const PositionType src_start_page = src_y / src_page_h;
        const PositionType src_end_page = (src_y + copy_h + src_page_h - 1) / src_page_h;

        for (PositionType p = src_start_page; p < src_end_page; ++p) {
            const PositionType src_page_y = p * src_page_h;
            const PositionType src_row_begin = kf::max(src_y, src_page_y);
            const PositionType src_row_end = kf::min(src_y + copy_h, src_page_y + src_page_h);
            const PositionType rows = src_row_end - src_row_begin;
            if (rows <= 0) { continue; }

            const u8 src_bit_offs = static_cast<u8>(src_row_begin - src_page_y);
            const PositionType dst_global_y = dst_y + (src_row_begin - src_y);
            const PositionType dst_page = dst_global_y / src_page_h;
            const u8 dst_bit_offs = static_cast<u8>(dst_global_y % src_page_h);

            const u8 dst_mask = static_cast<u8>(((1u << rows) - 1u) << dst_bit_offs);
            const u8 src_mask = static_cast<u8>(((1u << rows) - 1u) << src_bit_offs);

            for (PositionType x = 0; x < copy_w; ++x) {
                const PositionType src_col = src_x + x;
                const PositionType dst_col = dst_x + x;
                if (dst_col >= dst_stride) { continue; }

                const usize src_idx = static_cast<usize>(p) * src_w + src_col;
                if (src_idx >= src.length()) { continue; }

                u8 src_byte = src[src_idx];
                u8 src_bits = static_cast<u8>((src_byte & src_mask) >> src_bit_offs);
                src_bits = static_cast<u8>(src_bits << dst_bit_offs);

                const usize dst_idx = static_cast<usize>(dst_page) * dst_stride + dst_col;
                if (dst_idx >= dst.length()) { continue; }

                dst[dst_idx] = static_cast<u8>((dst[dst_idx] & ~dst_mask) | src_bits);
            }
        }
    }

private:
    /// @brief Calculate page mask for specified region
    /// @return Bit mask for the visible portion of the page
    static u8 calculatePageMask(int page, int offset_y, int height) noexcept {
        const auto page_top = (page * page_height);
        const auto page_bottom = (page_top + page_height - 1);

        const auto visible_top = kf::max(offset_y, page_top);
        const auto visible_bottom = (kf::min(offset_y + height, page_bottom + 1));

        if (visible_top >= visible_bottom) { return 0; }

        return createMask(
            static_cast<u8>(visible_top - page_top),
            static_cast<u8>(visible_bottom - page_top - 1));
    }

    /// @brief Create bit mask for specified bit range
    /// @return Bit mask covering [start_bit, end_bit] inclusive
    static constexpr u8 createMask(u8 start_bit, u8 end_bit) noexcept {
        if (start_bit > end_bit) {
            return 0;
        } else {
            return static_cast<u8>(((1 << (end_bit + 1)) - 1) ^ ((1 << start_bit) - 1));
        }
    }
};

}// namespace kf::pixel