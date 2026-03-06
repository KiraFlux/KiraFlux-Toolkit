// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "kf/Result.hpp"
#include "kf/aliases.hpp"
#include "kf/memory/Slice.hpp"

#include "kf/memory/io/tags.hpp"

namespace kf::memory::io {

template<typename Impl> struct Readable : ReadableTag {
    using ErrorImpl = typename Impl::Error;

    /// @brief Read single byte
    /// @return Byte value or error
    [[nodiscard]] Result<u8, ErrorImpl> readByte() noexcept {
        return impl().readByteImpl();
    }

    /// @brief Read arbitrary number of bytes into buffer
    /// @param buffer Destination buffer
    /// @return Slice containing actually read data, or error
    [[nodiscard]] Result<memory::Slice<u8>, ErrorImpl> readBuffer(memory::Slice<u8> buffer) noexcept {
        return impl().readBufferImpl(static_cast<const void *>(buffer.data()), buffer.size());
    }

    /// @brief Read fixed‑size buffer
    /// @tparam T Type of packet (trivially copyable)
    /// @return Packet value or error
    template<typename T> [[nodiscard]] Result<T, ErrorImpl> readPacket() noexcept {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

        T packet;
        return impl().readBufferImpl(static_cast<void *>(&packet), static_cast<usize>(sizeof(T)));
    }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }

    // Impl must provide:

    // [[nodiscard]] Result<u8, ErrorImpl> readByteimpl() noexcept
    // [[nodiscard]] Result<T, ErrorImpl> readBufferImpl(const void *dest, usize size) noexcept
};

}// namespace kf::memory::io