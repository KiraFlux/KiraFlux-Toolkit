// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Slice.hpp"
#include "kf/concepts.hpp"
#include "kf/math.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/pixel/Pixel.hpp"

namespace kf::image {

struct ImageTag {};

/// @brief Image
/// @tparam Impl Image implementation
/// @tparam P Pixel implementation
template<typename Impl, implements<pixel::PixelTag> P> struct Image :

    ImageTag,
    meta::CRTP<Impl>

{

    using BufferType = typename P::BufferType;

    // Abstract properties

    /// @brief Get current width in pixels (may differ from physical width due to orientation)
    [[nodiscard]] math::Pixels width() const noexcept {
        return this->impl().getWidthImpl();
    }

    /// @brief Get current height in pixels (may differ from physical width due to orientation)
    [[nodiscard]] math::Pixels height() const noexcept {
        return this->impl().getHeightImpl();
    }

    /// @brief Get current full width in pixels (may differ from physical width due to orientation)
    [[nodiscard]] math::Pixels stride() const noexcept {
        return this->impl().getStrideImpl();
    }

    /// @brief Get writable frame buffer
    [[nodiscard]] Slice<BufferType> buffer() noexcept {
        return this->impl().getBufferImpl();
    }

    /// @brief Get readonly frame buffer
    [[nodiscard]] Slice<const BufferType> buffer() const noexcept {
        return const_cast<Image *>(this)->impl().getBufferImpl();
    }

    // properties

    /// @brief Get maximum valid X coordinate
    [[nodiscard]] math::Pixels maxX() const noexcept {
        return width() - 1;
    }

    /// @brief Get maximum valid Y coordinate
    [[nodiscard]] math::Pixels maxY() const noexcept {
        return height() - 1;
    }

    /// @brief Get image size in bytes
    [[nodiscard]] usize size() const noexcept {
        return buffer().length() * sizeof(BufferType);
    }
};

}// namespace kf::image

#define KF_IMPL_IMAGE(...) friend struct ::kf::image::Image<__VA_ARGS__>
