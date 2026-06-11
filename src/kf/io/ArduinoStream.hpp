// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <Stream.h>

#include "kf/Result.hpp"
#include "kf/io/Readable.hpp"
#include "kf/io/Writable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

enum class ArduinoStreamError : u8 {
    ReadNotAvailable,
    ReadFalied,
    WriteFailed,
};

}

namespace kf::io {

struct ArduinoStream final :

    mixin::NonCopyable,
    io::Readable<ArduinoStream, internal::ArduinoStreamError>,
    io::Writable<ArduinoStream, kf::Result<void, internal::ArduinoStreamError>>

{
    using Error = internal::ArduinoStreamError;

    explicit constexpr ArduinoStream(Stream &stream) noexcept : _stream{stream} {}

private:
    Stream &_stream;

    KF_IMPL_READABLE(ArduinoStream, Error);

    auto readBufferImpl(Slice<u8> dest) noexcept -> Result<Slice<const u8>, Error> {
        constexpr auto min_available{1u};

        if (_stream.available() < min_available) { return error(Error::ReadNotAvailable); }

        const auto readed = _stream.readBytes(dest.data(), dest.size());
        if (readed < min_available) { return error(Error::ReadFalied); }

        return ok(Slice<const u8>{dest.data(), readed});
    }

    template<typename T> auto readPacketImpl() noexcept -> Result<T, Error> {
        constexpr auto to_read{sizeof(T)};
        if (_stream.available() < to_read) { return error(Error::ReadNotAvailable); }

        if constexpr (to_read == sizeof(u8)) {
            const auto read_result = _stream.read();
            if (-1 == read_result) {
                return error(Error::ReadFalied);
            } else {
                return ok(static_cast<T>(read_result));
            }
        } else {
            T dest;
            const auto readed = _stream.readBytes(reinterpret_cast<u8 *>(&dest), to_read);
            if (to_read == readed) {
                return ok(dest);
            } else {
                return error(Error::ReadFalied);
            }
        }
    }

    using WriteResult = Result<void, Error>;

    KF_IMPL_WRITABLE(ArduinoStream, WriteResult);

    WriteResult writeBufferImpl(Slice<const u8> buffer) noexcept {
        const auto to_write = buffer.size();
        // _stream.availableForWrite() ?
        if (_stream.write(buffer.data(), to_write) != to_write) { return error(Error::WriteFailed); }

        return ok();
    }

    template<typename T> WriteResult writePacketImpl(T &&packet) noexcept {
        constexpr auto to_write = sizeof(packet);
        usize written;

        if constexpr (to_write == sizeof(u8)) {
            written = _stream.write(static_cast<u8>(packet));
        } else {
            written = _stream.write(reinterpret_cast<const u8 *>(&packet), to_write);
        }

        if (to_write != written) { return error(Error::WriteFailed); }

        return ok();
    }

    template<typename T> WriteResult writeMixedImpl(T &&header, Slice<const u8> buffer) noexcept {
        const auto header_result = this->writePacket(std::forward<T>(header));
        if (header_result.isError()) { return header_result; }

        return this->writeBuffer(buffer);
    }
};

}// namespace kf::io