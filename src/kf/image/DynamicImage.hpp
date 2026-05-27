// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/Result.hpp"
#include "kf/image/Image.hpp"
#include "kf/math/units.hpp"
#include "kf/pixel/Pixel.hpp"

namespace kf::image {

/// @brief Dynamic display region with runtime dimensions
/// @tparam P Pixel implementation
template<typename P> struct DynamicImage final : Image<DynamicImage<P>, P> {
    KF_CHECK_IMPL(P, pixel::PixelTag);

    using PixelImpl = P;
    using BufferType = typename P::BufferType;
    using ColorType = typename P::ColorType;

    /// @brief Possible errors when creating DynamicImage
    enum class Error : u8 {
        BufferTooSmall,   ///< Buffer size too small for this region
        SizeTooSmall,     ///< Region dimensions are less than 1 pixel
        SizeTooLarge,     ///< Sub-region exceeds parent bounds
        OffsetOutOfBounds,///< Offset falls outside parent region
    };

    /// @brief Creates DynamicImage with validation
    [[nodiscard]] static Result<DynamicImage, Error> create(
        Slice<BufferType> buffer, math::Pixels stride,
        math::Pixels width, math::Pixels height,
        math::Pixels offset_x, math::Pixels offset_y) noexcept {

        if (buffer.size() >= PixelImpl::bufferSize(width, height)) { return Error::BufferTooSmall; }

        if (width < 1 or height < 1) { return Error::SizeTooSmall; }

        return DynamicImage{buffer, stride, width, height, offset_x, offset_y};
    }

    /// @brief Creates DynamicImage without validation
    /// @warning Caller must ensure parameters are valid
    explicit DynamicImage(
        Slice<BufferType> buffer, math::Pixels stride,
        math::Pixels width, math::Pixels height,
        math::Pixels offset_x, math::Pixels offset_y) noexcept :
        _buffer{buffer}, _stride{stride}, _width{width}, _height{height}, _offset_x{offset_x}, _offset_y{offset_y} {}

    template<typename I> explicit DynamicImage(I &image) noexcept :
        _buffer{image.buffer()}, _stride{image.stride()}, _width{image.width()}, _height{image.height()}, _offset_x{0}, _offset_y{0} {
        KF_CHECK_IMPL(I, image::ImageTag);
    }

    /// @brief Creates validated sub-region
    /// @return Sub-view or error if out of bounds
    [[nodiscard]] Result<DynamicImage, Error> sub(
        math::Pixels sub_width, math::Pixels sub_height,
        math::Pixels sub_offset_x, math::Pixels sub_offset_y) const noexcept {
        if (sub_offset_x >= _width or sub_offset_y >= _height) {
            return Error::OffsetOutOfBounds;
        }

        if (sub_width > _width - sub_offset_x or sub_height > _height - sub_offset_y) {
            return Error::SizeTooLarge;
        }

        return create(
            _buffer, _stride,
            sub_width, sub_height,
            static_cast<math::Pixels>(_offset_x + sub_offset_x),
            static_cast<math::Pixels>(_offset_y + sub_offset_y));
    }

    /// @brief Creates sub-region without validation
    /// @warning No bounds checking - caller must ensure parameters are valid
    [[nodiscard]] DynamicImage subUnchecked(
        math::Pixels sub_width, math::Pixels sub_height,
        math::Pixels sub_offset_x, math::Pixels sub_offset_y) noexcept {
        return DynamicImage{
            _buffer,
            _stride,
            sub_width,
            sub_height,
            static_cast<math::Pixels>(_offset_x + sub_offset_x),
            static_cast<math::Pixels>(_offset_y + sub_offset_y),
        };
    }

    /// @brief Checks if X coordinate is within view bounds
    [[nodiscard]] bool isInsideX(math::Pixels x_relative) const noexcept { return x_relative >= 0 and x_relative < _width; }

    /// @brief Checks if Y coordinate is within view bounds
    [[nodiscard]] bool isInsideY(math::Pixels y_relative) const noexcept { return y_relative >= 0 and y_relative < _height; }

    /// @brief Converts relative X to absolute buffer coordinate
    [[nodiscard]] math::Pixels toAbsoluteX(math::Pixels x) const noexcept { return static_cast<math::Pixels>(_offset_x + x); }

    /// @brief Converts relative Y to absolute buffer coordinate
    [[nodiscard]] math::Pixels toAbsoluteY(math::Pixels y) const noexcept { return static_cast<math::Pixels>(_offset_y + y); }

    /// @brief Sets single pixel color
    void setPixel(math::Pixels x_relative, math::Pixels y_relative, ColorType color) const noexcept {
        PixelImpl::setPixel(_buffer, _stride, toAbsoluteX(x_relative), toAbsoluteY(y_relative), color);
    }

    /// @brief Fills entire region with solid color
    void fill(ColorType color) const noexcept {
        PixelImpl::fill(_buffer, _stride, _offset_x, _offset_y, _width, _height, color);
    }

    /// @brief Fills rect region with solid color
    void fill(
        math::Pixels x0, math::Pixels y0,
        math::Pixels x1, math::Pixels y1,
        ColorType color) const noexcept {
        if (x0 > x1) { std::swap(x0, x1); }
        if (y0 > y1) { std::swap(y0, y1); }

        PixelImpl::fill(
            _buffer,
            _stride,
            static_cast<math::Pixels>(_offset_x + x0),
            static_cast<math::Pixels>(_offset_y + y0),
            static_cast<math::Pixels>(x1 - x0 + 1),
            static_cast<math::Pixels>(y1 - y0 + 1),
            color);
    }

private:
    Slice<BufferType> _buffer;        ///< display buffer memory view
    math::Pixels _width, _height;     ///< Region size in pixels
    math::Pixels _stride;             ///< Row stride (full display width)
    math::Pixels _offset_x, _offset_y;///< Absolute offset from buffer origin

    KF_IMPL(Image<DynamicImage<P>, P>);

    [[nodiscard]] constexpr math::Pixels getWidthImpl() const noexcept { return _width; }

    [[nodiscard]] constexpr math::Pixels getHeightImpl() const noexcept { return _height; }

    [[nodiscard]] constexpr math::Pixels getStrideImpl() const noexcept { return _stride; }

    [[nodiscard]] constexpr Slice<BufferType> getBufferImpl() noexcept { return _buffer; }
};

}// namespace kf::image