// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "kf/Result.hpp"
#include "kf/aliases.hpp"
#include "kf/memory/Slice.hpp"

#include "kf/memory/io/tags.hpp"

namespace kf::memory::io {

template<typename Impl, typename ErrorImpl> struct Readable : ReadableTag {

    /// @brief Read single byte
    /// @return Byte value or error
    [[nodiscard]] Result<u8, ErrorImpl> readByte() noexcept {
        return this->readPacket<u8>();
    }

    /// @brief Read arbitrary number of bytes into buffer
    /// @param buffer Destination buffer
    /// @return Slice containing actually read data, or error
    [[nodiscard]] Result<memory::Slice<const u8>, ErrorImpl> readBuffer(memory::Slice<u8> buffer) noexcept {
        return impl().readBufferImpl(buffer);
    }

    /// @brief Read fixed‑size buffer
    /// @tparam T Type of packet (trivially copyable)
    /// @return Packet value or error
    template<typename T> [[nodiscard]] Result<T, ErrorImpl> readPacket() noexcept {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        return impl().template readPacketImpl<T>();
    }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }

    // Impl must provide:

    // [[nodiscard]] Result<memory::Slice<const u8>, ErrorImpl> readBufferImpl(memory::Slice<u8> buffer) noexcept
    // template <typename T> [[nodiscard]] Result<T, ErrorImpl> readPacketImpl() noexcept
};

}// namespace kf::memory::io