// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "kf/aliases.hpp"
#include "kf/memory/Slice.hpp"

#include "kf/memory/io/tags.hpp"

namespace kf::memory::io {

/// @brief Writable Stream
/// @tparam Impl stream implementation
template<typename Impl> struct Writable : WritableTag {

    /// @brief Write single byte
    /// @return true if successful
    [[nodiscard]] bool writeByte(u8 byte) noexcept {
        return impl().writeByteImpl(byte);
    }

    /// @brief Write arbitrary data from buffer
    /// @param buffer Source data
    /// @return true if successful
    [[nodiscard]] bool writeBuffer(memory::Slice<const u8> buffer) noexcept {
        return impl().writeBufferImpl(static_cast<const void *>(buffer.data()), buffer.size());
    }

    /// @brief Write fixed‑size packet
    /// @tparam T Type of packet (trivially copyable)
    /// @param packet Value to write
    /// @return true if successful
    template<typename T> [[nodiscard]] bool writePacket(const T &packet) noexcept {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

        return impl().writeBufferImpl(static_cast<const void *>(&packet), static_cast<usize>(sizeof(T)));
    }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }

    // Impl must provide:

    // [[nodiscard]] bool writeByteImpl(u8 byte) noexcept
    // [[nodiscard]] bool writeBufferImpl(const void *source, usize size) noexcept
};

}// namespace kf::memory::io