// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math/units.hpp"
#include "kf/memory/Slice.hpp"

namespace kf::image {

/// @brief Image
/// @tparam Impl Image implementation
/// @tparam P Pixel implementation
template<typename Impl, typename P> struct Image {
    using PixelImpl = P;
    using BufferType = typename P::BufferType;
    using ColorType = typename P::ColorType;

    // Abstract properties

    /// @brief Get current width in pixels (may differ from physical width due to orientation)
    [[nodiscard]] Pixels width() const noexcept { return c_impl().getWidthImpl(); }

    /// @brief Get current height in pixels (may differ from physical width due to orientation)
    [[nodiscard]] Pixels height() const noexcept { return c_impl().getHeightImpl(); }

    /// @brief Get current full width in pixels (may differ from physical width due to orientation)
    [[nodiscard]] Pixels stride() const noexcept { return c_impl().getStrideImpl(); }

    /// @brief Get writable frame buffer
    [[nodiscard]] Slice<BufferType> buffer() noexcept { return impl().getBufferImpl(); }

    /// @brief Get readonly frame buffer
    [[nodiscard]] Slice<const BufferType> buffer() const noexcept { return c_impl().getBufferImpl(); }

    // properties

    /// @brief Get maximum valid X coordinate
    [[nodiscard]] u8 maxX() const noexcept { return width() - 1; }

    /// @brief Get maximum valid Y coordinate
    [[nodiscard]] u8 maxY() const noexcept { return height() - 1; }

    // CRTP
    friend Impl;

private:
    inline Impl &impl() noexcept { return *static_cast<Impl *>(this); }

    inline const Impl &c_impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::image