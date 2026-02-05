// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/pixel/PixelFormat.hpp"

namespace kf {// NOLINT(*-concat-nested-namespaces)
namespace pixel {

/// @brief 1-bit monochrome format (1 bit per pixel)
struct Monochrome final : PixelFormat<Monochrome, u8, bool, 1> {

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
                if (index < buffer.size()) {
                    buffer[index] = (buffer[index] & ~mask) | (fill_byte & mask);
                }
            }
        }
    }

    static void copyImpl(
        const Slice<BufferType> source_buffer,
        PositionType source_width,
        PositionType source_height,
        Slice<BufferType> dest_buffer,
        PositionType dest_stride,
        PositionType dest_width,
        PositionType dest_height,
        PositionType dest_x,
        PositionType dest_y) noexcept {
        // Boundary checks
        if (dest_x >= dest_width or dest_y >= dest_height) { return; }

        PositionType copy_width = source_width;
        PositionType copy_height = source_height;

        if (dest_x + copy_width > dest_width) {
            copy_width = dest_width - dest_x;
        }
        if (dest_y + copy_height > dest_height) {
            copy_height = dest_height - dest_y;
        }

        if (copy_width <= 0 or copy_height <= 0) { return; }

        const usize source_pages = (source_height + page_height - 1) / page_height;

        for (usize src_page = 0; src_page < source_pages; src_page++) {
            const PositionType src_y_start = src_page * page_height;
            const auto src_y_end = kf::min(int(src_y_start + page_height), int(copy_height));

            if (src_y_start >= src_y_end) { continue; }

            const usize rows_in_page = src_y_end - src_y_start;
            const PositionType dest_y_start = dest_y + src_y_start;
            const usize dest_page = dest_y_start / page_height;

            const u8 src_bit_offset = src_y_start % page_height;
            const u8 dest_bit_offset = dest_y_start % page_height;

            // Create destination bit mask
            u8 dest_mask = 0;
            for (usize i = 0; i < rows_in_page; i++) {
                dest_mask |= (1u << (dest_bit_offset + i));
            }

            for (PositionType x = 0; x < copy_width; x++) {
                const PositionType dest_col = dest_x + x;
                if (dest_col >= dest_stride) { continue; }

                // Read from source
                const usize src_index = src_page * source_width + x;
                if (src_index >= source_buffer.size()) {
                    continue;
                }
                const u8 src_byte = source_buffer[src_index];

                // Extract and shift bits
                u8 src_bits = 0;
                for (usize i = 0; i < rows_in_page; i++) {
                    if (src_byte & (1u << (src_bit_offset + i))) {
                        src_bits |= (1u << (dest_bit_offset + i));
                    }
                }

                // Write to destination
                const usize dest_index = dest_page * dest_stride + dest_col;
                if (dest_index < dest_buffer.size()) {
                    dest_buffer[dest_index] = (dest_buffer[dest_index] & ~dest_mask) | src_bits;
                }
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

}// namespace pixel
}// namespace kf