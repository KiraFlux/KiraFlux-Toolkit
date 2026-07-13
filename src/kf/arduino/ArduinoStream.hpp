// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include <Stream.h>

#include "kf/Result.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Readable.hpp"
#include "kf/mixin/Writable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

enum class ArduinoStreamError : u8 {
    ReadNotAvailable,
    ReadFailed,
    WriteFailed,
};

}

namespace kf::arduino {

struct ArduinoStream final :

    mixin::NonCopyable,
    mixin::Readable<ArduinoStream, internal::ArduinoStreamError>,
    mixin::Writable<ArduinoStream, kf::Result<void, internal::ArduinoStreamError>>

{
    using Error = internal::ArduinoStreamError;

    explicit constexpr ArduinoStream(Stream &stream) noexcept :
        _stream{stream} {}

private:
    Stream &_stream;

    KF_IMPL_READABLE(ArduinoStream, Error);

    auto readBufferImpl(Slice<u8> dest) noexcept -> Result<Slice<const u8>, Error> {
        constexpr auto min_available{1u};

        if (_stream.available() < min_available) { return error(Error::ReadNotAvailable); }

        const auto bytes_read = _stream.readBytes(dest.data(), dest.length());
        if (bytes_read < min_available) { return error(Error::ReadFailed); }

        return ok(Slice<const u8>{dest.data(), bytes_read});
    }

    template<typename T> auto readPacketImpl() noexcept -> Result<T, Error> {
        constexpr auto to_read{sizeof(T)};
        if (_stream.available() < to_read) { return error(Error::ReadNotAvailable); }

        if constexpr (to_read == sizeof(u8)) {
            const auto read_result = _stream.read();
            if (-1 == read_result) {
                return error(Error::ReadFailed);
            } else {
                return ok(static_cast<T>(read_result));
            }
        } else {
            T dest;
            const auto bytes_read = _stream.readBytes(reinterpret_cast<u8 *>(&dest), to_read);
            if (to_read == bytes_read) {
                return ok(dest);
            } else {
                return error(Error::ReadFailed);
            }
        }
    }

    using WriteResult = Result<void, Error>;

    KF_IMPL_WRITABLE(ArduinoStream, WriteResult);

    WriteResult writeBufferImpl(Slice<const u8> buffer) noexcept {
        const auto to_write = buffer.length();
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
        KF_TRY(this->writePacket(std::forward<T>(header)));
        return this->writeBuffer(buffer);
    }
};

}// namespace kf::arduino