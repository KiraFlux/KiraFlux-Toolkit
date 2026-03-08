// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "kf/Result.hpp"
#include "kf/aliases.hpp"
#include "kf/memory/Slice.hpp"

#include "kf/memory/io/tags.hpp"

namespace kf::memory::io {

/// @brief Writable Stream
/// @tparam Impl stream implementation
template<typename Impl> struct Writable : WritableTag {
    using ErrorImpl = typename Impl::Error;

    /// @brief Write single byte
    /// @return Result indicating success or error
    [[nodiscard]] Result<void, ErrorImpl> writeByte(u8 byte) noexcept {
        return this->writePacket(byte);
    }

    /// @brief Write arbitrary data from buffer
    /// @param buffer Source data
    /// @return Result indicating success or error
    [[nodiscard]] Result<void, ErrorImpl> writeBuffer(memory::Slice<const u8> buffer) noexcept {
        return impl().writeBufferImpl(buffer);
    }

    /// @brief Write fixed‑size packet
    /// @tparam T Type of packet (trivially copyable)
    /// @param packet Value to write
    /// @return Result indicating success or error
    template<typename T> [[nodiscard]] Result<void, ErrorImpl> writePacket(const T &packet) noexcept {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        return impl().writePacketImpl(packet);
    }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }

    // Impl must provide:

    // [[nodiscard]] Result<void, Error> writePacketImpl(T packet) noexcept // may use method overload
    // [[nodiscard]] Result<void, Error> writeBufferImpl(memory::Slice<const u8> buffer) noexcept
};

}// namespace kf::memory::io