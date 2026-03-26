// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>
#include <utility>

#include "kf/Result.hpp"
#include "kf/aliases.hpp"
#include "kf/memory/Slice.hpp"

namespace kf::memory::io {

struct WritableTag {};

/// @brief CRTP base class for writable streams.
/// @tparam Impl      Derived class.
/// @tparam ErrorImpl Error type used by the implementation.
/// @note Derived classes must implement:
///
///       - `Result<void, Error> writeBufferImpl(memory::Slice<const u8> buffer) noexcept`
///         Write a contiguous buffer of bytes.
///
///       - `template<typename T> Result<void, Error> writePacketImpl(T &&packet) noexcept`
///         Write a trivially copyable object.
///
///       - `template<typename T> Result<void, Error> writeMixedImpl(T &&header, memory::Slice<const u8> buffer) noexcept`
///         Write a small header followed by a buffer (e.g. command + data) in one transaction.
template<typename Impl, typename ErrorImpl> struct Writable : WritableTag {

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
    template<typename T> [[nodiscard]] Result<void, ErrorImpl> writePacket(T &&packet) noexcept {
        static_assert(std::is_trivially_copyable_v<std::decay_t<T>>, "T must be trivially copyable");
        return impl().writePacketImpl(std::forward<T>(packet));
    }

    /// @brief Write mixed packet: fixed-size header and dymanic-sized buffer
    /// @tparam T Type of packet (trivially copyable)
    /// @param header Header to write
    /// @param buffer Source buffer
    /// @return Result indicating success or error
    template<typename T> [[nodiscard]] Result<void, ErrorImpl> writeMixed(T &&header, memory::Slice<const u8> buffer) noexcept {
        static_assert(std::is_trivially_copyable_v<std::decay_t<T>>, "T must be trivially copyable");
        return impl().writeMixedImpl(std::forward<T>(header), buffer);
    }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::memory::io

#define KF_IMPL_WRITABLE(__impl__, __error_type__) friend struct kf::memory::io::Writable<__impl__, __error_type__>