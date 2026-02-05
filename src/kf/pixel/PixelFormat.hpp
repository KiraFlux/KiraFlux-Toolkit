// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/attributes.hpp"
#include "kf/aliases.hpp"


namespace kf { // NOLINT(*-concat-nested-namespaces) //c++11 capability
namespace pixel {

template<typename I, typename Tb, typename Tc, u8 bits> struct PixelFormat {
    // types

    using PositionType = u16;
    using BufferType = Tb;
    using ColorType = Tc;

    // values

    static constexpr auto bits_per_pixel{bits};

    template<PositionType W, PositionType H> static constexpr usize buffer_size{I::getBufferSizeImpl(W, H)};

    // conventions

    kf_nodiscard static constexpr ColorType fromRgb(
        u8 r,
        u8 g,
        u8 b
    ) noexcept { return I::fromRgbImpl(r, g, b); }

    // draw

    /// @brief set pixel color
    static void setPixel(
        BufferType *buffer,
        PositionType stride,
        PositionType abs_x,
        PositionType abs_y,
        ColorType color
    ) noexcept {
        I::setPixelImpl(
            buffer, stride,
            abs_x, abs_y,
            color
        );
    }

    /// @brief Effective fill rectangular region with specified color
    static void fill(
        BufferType *buffer,
        PositionType stride,
        PositionType offset_x,
        PositionType offset_y,
        PositionType width,
        PositionType height,
        ColorType color
    ) noexcept {
        I::fillImpl(
            buffer, stride,
            offset_x, offset_y,
            width, height,
            color
        );
    }

    /// @brief Copy rectangular region from source to destination buffer
    static void copy(
        // source buffer
        const BufferType *source_buffer, //(stride = width)
        PositionType source_width,
        PositionType source_height,

        // dest buffer
        BufferType *dest_buffer,
        PositionType dest_stride,
        PositionType dest_width,
        PositionType dest_height,

        // dest paste position
        PositionType dest_paste_abs_x,
        PositionType dest_paste_abs_y
    ) noexcept {
        I::copyImpl(
            source_buffer, source_width, source_height,
            dest_buffer, dest_stride, dest_width, dest_height,
            dest_paste_abs_x, dest_paste_abs_y
        );
    }


    // CRTP

    friend I;
protected:
    using Base = PixelFormat;
};

}
}