// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    image/DynamicImage.hpp
/// @brief   Image with runtime dimensions and sub‑region support.

#pragma once

#include <utility>

#include "kf/Result.hpp"
#include "kf/concepts.hpp"
#include "kf/image/Image.hpp"
#include "kf/pixel/Pixel.hpp"
#include "kf/units.hpp"

namespace kf::image {

/// @brief Dynamic display region with runtime dimensions
/// @tparam P Pixel implementation
template<implements<pixel::PixelTag> P> struct DynamicImage final : Image<DynamicImage<P>, P> {

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
        Slice<BufferType> buffer, units::Pixels stride,
        units::Pixels width, units::Pixels height,
        units::Pixels offset_x, units::Pixels offset_y) noexcept {

        if (buffer.size() >= PixelImpl::bufferSize(width, height)) { return Error::BufferTooSmall; }

        if (width < 1 or height < 1) { return Error::SizeTooSmall; }

        return DynamicImage{buffer, stride, width, height, offset_x, offset_y};
    }

    /// @brief Creates DynamicImage without validation
    /// @warning Caller must ensure parameters are valid
    explicit DynamicImage(
        Slice<BufferType> buffer, units::Pixels stride,
        units::Pixels width, units::Pixels height,
        units::Pixels offset_x, units::Pixels offset_y) noexcept :
        _buffer{buffer}, _stride{stride}, _width{width}, _height{height}, _offset_x{offset_x}, _offset_y{offset_y} {}

    template<implements<image::ImageTag> I> explicit DynamicImage(I &image) noexcept :
        _buffer{image.buffer()}, _stride{image.stride()}, _width{image.width()}, _height{image.height()}, _offset_x{0}, _offset_y{0} {}

    /// @brief Creates validated sub-region
    /// @return Sub-view or error if out of bounds
    [[nodiscard]] auto sub(
        units::Pixels sub_width, units::Pixels sub_height,
        units::Pixels sub_offset_x, units::Pixels sub_offset_y) const noexcept -> Result<DynamicImage, Error> {
        if (sub_offset_x >= _width or sub_offset_y >= _height) {
            return Error::OffsetOutOfBounds;
        }

        if (sub_width > _width - sub_offset_x or sub_height > _height - sub_offset_y) {
            return Error::SizeTooLarge;
        }

        return create(
            _buffer, _stride,
            sub_width, sub_height,
            static_cast<units::Pixels>(_offset_x + sub_offset_x),
            static_cast<units::Pixels>(_offset_y + sub_offset_y));
    }

    /// @brief Creates sub-region without validation
    /// @warning No bounds checking - caller must ensure parameters are valid
    [[nodiscard]] DynamicImage subUnchecked(
        units::Pixels sub_width, units::Pixels sub_height,
        units::Pixels sub_offset_x, units::Pixels sub_offset_y) noexcept {
        return DynamicImage{
            _buffer,
            _stride,
            sub_width,
            sub_height,
            static_cast<units::Pixels>(_offset_x + sub_offset_x),
            static_cast<units::Pixels>(_offset_y + sub_offset_y),
        };
    }

    /// @brief Checks if X coordinate is within view bounds
    [[nodiscard]] bool isInsideX(units::Pixels x_relative) const noexcept {
        return x_relative >= 0 and x_relative < _width;
    }

    /// @brief Checks if Y coordinate is within view bounds
    [[nodiscard]] bool isInsideY(units::Pixels y_relative) const noexcept {
        return y_relative >= 0 and y_relative < _height;
    }

    /// @brief Converts relative X to absolute buffer coordinate
    [[nodiscard]] units::Pixels toAbsoluteX(units::Pixels x) const noexcept {
        return static_cast<units::Pixels>(_offset_x + x);
    }

    /// @brief Converts relative Y to absolute buffer coordinate
    [[nodiscard]] units::Pixels toAbsoluteY(units::Pixels y) const noexcept {
        return static_cast<units::Pixels>(_offset_y + y);
    }

    /// @brief Sets single pixel color
    void setPixel(units::Pixels x_relative, units::Pixels y_relative, ColorType color) const noexcept {
        PixelImpl::setPixel(_buffer, _stride, toAbsoluteX(x_relative), toAbsoluteY(y_relative), color);
    }

    /// @brief Fills entire region with solid color
    void fill(ColorType color) const noexcept {
        PixelImpl::fill(_buffer, _stride, _offset_x, _offset_y, _width, _height, color);
    }

    /// @brief Fills rect region with solid color
    void fill(
        units::Pixels x0, units::Pixels y0,
        units::Pixels x1, units::Pixels y1,
        ColorType color) const noexcept {
        if (x0 > x1) { std::swap(x0, x1); }
        if (y0 > y1) { std::swap(y0, y1); }

        PixelImpl::fill(
            _buffer,
            _stride,
            static_cast<units::Pixels>(_offset_x + x0),
            static_cast<units::Pixels>(_offset_y + y0),
            static_cast<units::Pixels>(x1 - x0 + 1),
            static_cast<units::Pixels>(y1 - y0 + 1),
            color);
    }

private:
    Slice<BufferType> _buffer;         ///< display buffer memory view
    units::Pixels _width, _height;     ///< Region size in pixels
    units::Pixels _stride;             ///< Row stride (full display width)
    units::Pixels _offset_x, _offset_y;///< Absolute offset from buffer origin

    KF_IMPL_IMAGE(DynamicImage<P>, P);

    constexpr units::Pixels getWidthImpl() const noexcept { return _width; }

    constexpr units::Pixels getHeightImpl() const noexcept { return _height; }

    constexpr units::Pixels getStrideImpl() const noexcept { return _stride; }

    constexpr Slice<BufferType> getBufferImpl() noexcept { return _buffer; }
};

}// namespace kf::image