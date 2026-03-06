// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include <Stream.h>

#include "kf/Option.hpp"
#include "kf/aliases.hpp"

namespace kf::memory::io {

/// @brief Input stream for reading data from Stream interface
/// @note Wraps Arduino Stream class for type-safe reading with error handling
struct InputStream {

private:
    Stream &_stream;///< Reference to underlying Stream object

public:
    /// @brief Construct input stream from Stream reference
    /// @param s Stream object to read from
    explicit InputStream(Stream &s) noexcept :
        _stream{s} {}

    /// @brief Discard all available data in stream
    void clean() noexcept {
        while (_stream.available()) {
            (void) _stream.read();
        }
    }

    /// @brief Get number of bytes available for reading
    /// @return Count of bytes available in stream
    [[nodiscard]] usize available() noexcept {
        return _stream.available();
    }

    /// @brief Read single byte from stream
    /// @return Optional byte value (empty if no data available)
    [[nodiscard]] Option<u8> readByte() noexcept {
        constexpr auto arduino_stream_read_failed{-1};
        const auto result = _stream.read();

        if (result != arduino_stream_read_failed) {
            return {static_cast<u8>(result)};
        } else {
            return {};
        }
    }

    /// @brief Read object of type T from stream
    /// @tparam T Type to read (must be trivially copyable)
    /// @return Optional object (empty if insufficient data available)
    template<typename T> [[nodiscard]] Option<T> read() noexcept {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

        T value;

        if (_stream.readBytes(reinterpret_cast<u8 *>(&value), sizeof(T)) == sizeof(T)) {
            return {value};
        } else {
            return {};
        }
    }
};

}// namespace kf::io