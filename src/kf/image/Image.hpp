// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math/units.hpp"
#include "kf/memory/Slice.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/pixel/Pixel.hpp"

namespace kf::image {

struct ImageTag {};

/// @brief Image
/// @tparam Impl Image implementation
/// @tparam P Pixel implementation
template<typename Impl, typename P> struct Image : ImageTag {
    kf_crtp_check(P, pixel::PixelTag);

    using PixelImpl = P;
    using BufferType = typename P::BufferType;
    using ColorType = typename P::ColorType;

    // Abstract properties

    /// @brief Get current width in pixels (may differ from physical width due to orientation)
    [[nodiscard]] math::Pixels width() const noexcept { return impl().getWidthImpl(); }

    /// @brief Get current height in pixels (may differ from physical width due to orientation)
    [[nodiscard]] math::Pixels height() const noexcept { return impl().getHeightImpl(); }

    /// @brief Get current full width in pixels (may differ from physical width due to orientation)
    [[nodiscard]] math::Pixels stride() const noexcept { return impl().getStrideImpl(); }

    /// @brief Get writable frame buffer
    [[nodiscard]] memory::Slice<BufferType> buffer() noexcept { return impl().getBufferImpl(); }

    /// @brief Get readonly frame buffer
    [[nodiscard]] memory::Slice<const BufferType> buffer() const noexcept { return impl().getBufferImpl(); }

    // properties

    /// @brief Get maximum valid X coordinate
    [[nodiscard]] u8 maxX() const noexcept { return width() - 1; }

    /// @brief Get maximum valid Y coordinate
    [[nodiscard]] u8 maxY() const noexcept { return height() - 1; }

    /// @brief Get image size in bytes
    [[nodiscard]] usize size() const noexcept { return buffer().size() * sizeof(BufferType); }

    // CRTP
    friend Impl;

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::image