#pragma once

#include <Stream.h>
#include <kf/Option.hpp>
#include <kf/aliases.hpp>

namespace bytelang::core {

/// Входной поток (чтение данных)
struct InputStream {

private:
    Stream &stream;

public:
    explicit InputStream(Stream &s) :
        stream{s} {}

    void clean() {
        while (stream.available()) {
            (void) stream.read();
        }
    }

    /// Сколько байт доступно для чтения
    [[nodiscard]] kf::usize available() {
        return stream.available();
    }

    /// Прочитать один байт
    [[nodiscard]] kf::Option<kf::u8> readByte() {
        const auto result = stream.read();

        if (result == -1) {
            return {};
        } else {
            return {static_cast<kf::u8>(result)};
        }
    }

    /// Прочитать объект типа T
    template<typename T> [[nodiscard]] kf::Option<T> read() {
        T value;

        const kf::usize bytes_read = stream.readBytes(
            reinterpret_cast<kf::u8 *>(&value),
            sizeof(T));

        if (bytes_read == sizeof(T)) {
            return {value};
        } else {
            return {};
        }
    }
};

/// Выходной поток (запись данных)
struct OutputStream {

private:
    Stream &stream;

public:
    explicit OutputStream(Stream &s) :
        stream{s} {}

    /// Записать один байт
    [[nodiscard]] bool writeByte(kf::u8 byte) {
        return stream.write(byte) == 1;
    }

    /// Записать буфер
    [[nodiscard]] bool write(const void *data, kf::usize length) {
        return stream.write(static_cast<const kf::u8 *>(data), length) == length;
    }

    /// Записать объект типа T
    template<typename T> [[nodiscard]] inline bool write(const T &value) {
        return write(static_cast<const void *>(&value), sizeof(T));
    }
};

}// namespace bytelang::core
