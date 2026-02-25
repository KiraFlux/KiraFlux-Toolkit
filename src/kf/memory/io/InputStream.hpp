// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Stream.h>

#include "kf/Option.hpp"
#include "kf/aliases.hpp"
#include "kf/attributes.hpp"

namespace kf::io {

/// @brief Input stream for reading data from Stream interface
/// @note Wraps Arduino Stream class for type-safe reading with error handling
struct InputStream {

private:
    Stream &stream;///< Reference to underlying Stream object

public:
    /// @brief Construct input stream from Stream reference
    /// @param s Stream object to read from
    explicit InputStream(Stream &s) noexcept :
        stream{s} {}

    /// @brief Discard all available data in stream
    void clean() noexcept {
        while (stream.available()) {
            (void) stream.read();
        }
    }

    /// @brief Get number of bytes available for reading
    /// @return Count of bytes available in stream
    [[nodiscard]] usize available() noexcept {
        return stream.available();
    }

    /// @brief Read single byte from stream
    /// @return Optional byte value (empty if no data available)
    [[nodiscard]] Option<u8> readByte() noexcept {
        const auto result = stream.read();

        if (result == -1) {
            return {};
        } else {
            return {static_cast<u8>(result)};
        }
    }

    /// @brief Read object of type T from stream
    /// @tparam T Type to read (must be trivially copyable)
    /// @return Optional object (empty if insufficient data available)
    template<typename T> [[nodiscard]] Option<T> read() noexcept {
        T value;

        const usize bytes_read = stream.readBytes(
            reinterpret_cast<u8 *>(&value),
            sizeof(T));

        if (bytes_read == sizeof(T)) {
            return {value};
        } else {
            return {};
        }
    }
};

}// namespace kf::io