// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"
#include "kf/core/PixelFormat.hpp"
#include "kf/core/aliases.hpp"
#include "kf/math/units.hpp"

namespace kf {

/// @brief Pixel traits template for different pixel formats
/// @tparam Format Pixel format to provide traits for
template<PixelFormat Format> struct PixelTraits;

/// @brief Monochrome pixel format traits (1 bit per pixel)
/// @details Provides buffer type, color type, and operations for monochrome displays
template<> struct PixelTraits<PixelFormat::Monochrome> {
    using BufferType = u8; ///< Buffer element type (uint8_t)
    using ColorType = bool;///< Color representation type (bool)

    /// @brief Default foreground color (white/on)
    static constexpr ColorType foreground_default{true};

    /// @brief Default background color (black/off)
    static constexpr ColorType background_default{false};

    static constexpr u8 bits_per_pixel = 1;///< Bits per pixel
    static constexpr u8 page_height = 8;   ///< Vertical pixels per memory page

    /// @brief Calculate buffer size for given dimensions
    /// @tparam W Width in pixels
    /// @tparam H Height in pixels
    /// @return Required buffer size in bytes
    template<usize W, usize H> static constexpr usize buffer_size = ((W * H) + 7) / 8;

    /// @brief Calculate number of memory pages for given height
    /// @tparam H Height in pixels
    /// @return Number of 8-pixel memory pages
    template<usize H> static constexpr usize pages = (H + 7) / 8;

    /// @brief Set pixel value in monochrome buffer
    /// @param buffer Pointer to display buffer
    /// @param stride Buffer stride (width in pixels)
    /// @param abs_x Absolute X coordinate
    /// @param abs_y Absolute Y coordinate
    /// @param on Pixel state (true = on, false = off)
    static void setPixel(
        BufferType *buffer,
        Pixel stride,
        Pixel abs_x,
        Pixel abs_y,
        ColorType on) noexcept {
        const auto page = static_cast<Pixel>(abs_y / page_height);
        const auto bit_mask = static_cast<u8>(1 << (abs_y % page_height));
        const usize index = page * stride + abs_x;

        if (on) {
            buffer[index] |= bit_mask;
        } else {
            buffer[index] &= ~bit_mask;
        }
    }

    /// @brief Fill rectangular region with specified value
    /// @param buffer Pointer to display buffer
    /// @param stride Buffer stride (width in pixels)
    /// @param offset_x Region X offset
    /// @param offset_y Region Y offset
    /// @param width Region width in pixels
    /// @param height Region height in pixels
    /// @param value Fill value (true = on, false = off)
    static void fill(
        BufferType *buffer,
        Pixel stride,
        Pixel offset_x,
        Pixel offset_y,
        Pixel width,
        Pixel height,
        ColorType value) noexcept {
        const auto start_page = static_cast<Pixel>(offset_y / page_height);
        const auto end_page = static_cast<Pixel>((offset_y + height + page_height - 1) / page_height);
        const u8 fill_byte = value ? 0xFF : 0x00;

        for (Pixel page = start_page; page < end_page; page += 1) {
            const u8 mask = calculatePageMask(page, offset_y, height);
            if (mask == 0) { continue; }

            for (Pixel x = 0; x < width; x += 1) {
                const auto abs_x = offset_x + x;
                if (abs_x < 0 or abs_x >= stride) { continue; }

                const usize index = page * stride + abs_x;
                buffer[index] = (buffer[index] & ~mask) | (fill_byte & mask);
            }
        }
    }

    /// @brief Copy rectangular region from source to destination buffer
    /// @param source_buffer Source buffer pointer
    /// @param source_width Source region width in pixels
    /// @param source_height Source region height in pixels
    /// @param dest_buffer Destination buffer pointer
    /// @param dest_stride Destination buffer stride (width in pixels)
    /// @param dest_width Destination buffer width in pixels
    /// @param dest_height Destination buffer height in pixels
    /// @param dest_x Destination X coordinate
    /// @param dest_y Destination Y coordinate
    static void copy(
        const BufferType *source_buffer,
        Pixel source_width,
        Pixel source_height,
        BufferType *dest_buffer,
        Pixel dest_stride,
        Pixel dest_width,
        Pixel dest_height,
        Pixel dest_x,
        Pixel dest_y) noexcept {

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

        const usize source_pages = (source_height + page_height - 1) / page_height;

        for (auto src_page = 0; src_page < source_pages; src_page += 1) {
            const auto src_y_start = src_page * page_height;
            const auto src_y_end = kf::min(src_y_start + page_height, copy_height);

            if (src_y_start >= src_y_end) { continue; }

            const usize rows_in_page = src_y_end - src_y_start;
            const auto dest_y_start = dest_y + src_y_start;
            const usize dest_page = dest_y_start / page_height;

            const u8 src_bit_offset = static_cast<u8>(src_y_start % page_height);
            const u8 dest_bit_offset = static_cast<u8>(dest_y_start % page_height);

            // Destination bit mask
            u8 dest_mask = 0;
            for (usize i = 0; i < rows_in_page; i += 1) {
                dest_mask |= static_cast<u8>(1 << (dest_bit_offset + i));
            }

            for (auto x = 0; x < copy_width; x += 1) {
                const auto dest_col = dest_x + x;
                if (dest_col >= dest_stride) { continue; }

                // Read from source
                const usize src_index = src_page * source_width + x;
                const u8 src_byte = source_buffer[src_index];

                // Extract and shift bits
                u8 src_bits = 0;
                for (usize i = 0; i < rows_in_page; i += 1) {
                    if (src_byte & static_cast<u8>(1 << (src_bit_offset + i))) {
                        src_bits |= static_cast<u8>(1 << (dest_bit_offset + i));
                    }
                }

                // Write to destination (clear old bits first)
                const usize dest_index = dest_page * dest_stride + dest_col;
                dest_buffer[dest_index] = (dest_buffer[dest_index] & ~dest_mask) | (src_bits & dest_mask);
            }
        }
    }

private:
    /// @brief Calculate page mask for specified region
    /// @param page Page number
    /// @param offset_y Region Y offset
    /// @param height Region height in pixels
    /// @return Bit mask for the visible portion of the page
    static u8 calculatePageMask(Pixel page, Pixel offset_y, Pixel height) noexcept {
        const auto page_top = static_cast<Pixel>(page * page_height);
        const auto page_bottom = static_cast<Pixel>(page_top + page_height - 1);

        const Pixel visible_top = kf::max(offset_y, page_top);
        const auto visible_bottom = static_cast<Pixel>(kf::min(offset_y + height, page_bottom + 1));

        if (visible_top >= visible_bottom) { return 0; }

        return createMask(
            static_cast<u8>(visible_top - page_top),
            static_cast<u8>(visible_bottom - page_top - 1));
    }

    /// @brief Create bit mask for specified bit range
    /// @param start_bit Starting bit (0-7)
    /// @param end_bit Ending bit (0-7, must be >= start_bit)
    /// @return Bit mask covering [start_bit, end_bit] inclusive
    static constexpr u8 createMask(u8 start_bit, u8 end_bit) noexcept {
        if (start_bit > end_bit) {
            return 0;
        } else {
            return static_cast<u8>(((1 << (end_bit + 1)) - 1) ^ ((1 << start_bit) - 1));
        }
    }
};

/// @brief RGB565 pixel format traits (16 bits per pixel)
/// @details Provides buffer type, color type, and operations for RGB565 displays
template<> struct PixelTraits<PixelFormat::RGB565> {
    using BufferType = u16;///< Buffer element type (u16)
    using ColorType = u16; ///< Color representation type (RGB565 format)

    /// @brief Default foreground color (white)
    static constexpr ColorType foreground_default{0xFFFF};

    /// @brief Default background color (black)
    static constexpr ColorType background_default{0x0000};

    static constexpr u8 bits_per_pixel = 16;///< Bits per pixel

    /// @brief Calculate buffer size for given dimensions
    /// @tparam W Width in pixels
    /// @tparam H Height in pixels
    /// @return Required buffer size in elements (W * H)
    template<usize W, usize H> static constexpr usize buffer_size = W * H;

    /// @brief Set pixel value in RGB565 buffer
    /// @param buffer Pointer to display buffer
    /// @param stride Buffer stride (width in pixels)
    /// @param abs_x Absolute X coordinate
    /// @param abs_y Absolute Y coordinate
    /// @param color RGB565 color value
    static void setPixel(
        BufferType *buffer,
        Pixel stride,
        Pixel abs_x,
        Pixel abs_y,
        ColorType color) noexcept {
        buffer[abs_y * stride + abs_x] = color;
    }

    /// @brief Fill rectangular region with specified color
    /// @param buffer Pointer to display buffer
    /// @param stride Buffer stride (width in pixels)
    /// @param offset_x Region X offset
    /// @param offset_y Region Y offset
    /// @param width Region width in pixels
    /// @param height Region height in pixels
    /// @param color RGB565 fill color
    static void fill(
        BufferType *buffer,
        Pixel stride,
        Pixel offset_x,
        Pixel offset_y,
        Pixel width,
        Pixel height,
        ColorType color) noexcept {
        for (usize y = 0; y < height; y += 1) {
            const auto abs_y = offset_y + y;
            const usize row_start = abs_y * stride + offset_x;

            for (usize x = 0; x < width; x += 1) {
                buffer[row_start + x] = color;
            }
        }
    }

    /// @brief Copy rectangular region from source to destination buffer
    /// @param source_buffer Source buffer pointer
    /// @param source_width Source region width in pixels
    /// @param source_height Source region height in pixels
    /// @param dest_buffer Destination buffer pointer
    /// @param dest_stride Destination buffer stride (width in pixels)
    /// @param dest_width Destination buffer width in pixels
    /// @param dest_height Destination buffer height in pixels
    /// @param dest_x Destination X coordinate
    /// @param dest_y Destination Y coordinate
    static void copy(
        const BufferType *source_buffer,
        Pixel source_width,
        Pixel source_height,
        BufferType *dest_buffer,
        Pixel dest_stride,
        Pixel dest_width,
        Pixel dest_height,
        Pixel dest_x,
        Pixel dest_y) noexcept {

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

}// namespace kf
