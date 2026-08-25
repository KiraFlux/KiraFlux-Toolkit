// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    main.hpp
/// @brief   Defines the application entry point and the Init context (logger, arena, I/O).

#pragma once

#include "kf/Arena.hpp"
#include "kf/Logger.hpp"
#include "kf/core.hpp"

// autoconfig

#ifndef KF_CONFIG_MAIN_ARENA_BUFFER_LENGTH
#define KF_CONFIG_MAIN_ARENA_BUFFER_LENGTH (1024 * 16)
#endif

#ifndef KF_CONFIG_MAIN_LOGGER_KEY
#define KF_CONFIG_MAIN_LOGGER_KEY "main"
#endif

#ifndef KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH
#define KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH 256
#endif

#ifndef KF_CONFIG_MAIN_UART_BAUDRATE
#define KF_CONFIG_MAIN_UART_BAUDRATE 115200
#endif

#ifndef KF_CONFIG_MAIN_UART_NUM
#define KF_CONFIG_MAIN_UART_NUM 0
#endif

//

namespace kf {

/// @brief Application Context
struct Init;

/// @brief KiraFlux Toolkit application entry point
void main(Init &init);

namespace internal {

struct InitBase {

    /// @brief Application Arena allocator
    Arena arena{
        {_main_arena_buffer},
    };

    /// @brief Application Logger
    Logger logger{
        (KF_CONFIG_MAIN_LOGGER_KEY),
        {_main_logger_buffer},
    };

private:
    u8 _main_arena_buffer[(KF_CONFIG_MAIN_ARENA_BUFFER_LENGTH)]{};
    char _main_logger_buffer[(KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH)]{};
};

}// namespace internal

}// namespace kf

#ifdef ARDUINO

#include <Arduino.h>

#include "kf/UART.hpp"

namespace kf {

struct Init : internal::InitBase {

    /// @brief Platform-depended standard binary Input/Output Stream
    UART io{
        _uart_config,
        (KF_CONFIG_MAIN_UART_NUM),
        gpio::G3,// RX
        gpio::G1,// TX
    };

private:
    UART::Config _uart_config{
        .baudrate{KF_CONFIG_MAIN_UART_BAUDRATE},
    };
};

}// namespace kf

#define KF_PLATFORM_APP_FUNCTION_DECLARATION void setup()
#define KF_PLATFORM_APP_QUIT()

void loop() {}// not used

#else

#include <csignal>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>

#include "kf/Bytes.hpp"
#include "kf/BytesView.hpp"
#include "kf/Result.hpp"

#include "kf/mixin/BinaryReadable.hpp"
#include "kf/mixin/BinaryWritable.hpp"
#include "kf/mixin/Flush.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"
#include "kf/mixin/ReadAvailable.hpp"
#include "kf/mixin/WriteAvailable.hpp"

#define KF_PLATFORM_APP_FUNCTION_DECLARATION int main()
#define KF_PLATFORM_APP_QUIT() (0)

namespace kf {

namespace internal {

enum class AppIoError : char {
    ReadFailed,
    WriteFailed,
};

using AppIoWriteResult = Result<void, AppIoError>;

struct AppIO :

    mixin::NonCopyable,
    mixin::Initable<AppIO, void()>,
    mixin::Quitable<AppIO>,

    mixin::BinaryReadable<AppIO, AppIoError>,
    mixin::ReadAvailable<AppIO>,
    mixin::BinaryWritable<AppIO, AppIoWriteResult>,
    mixin::WriteAvailable<AppIO>,

    mixin::Flush<AppIO>

{
    using Self = AppIO;

    using Error = AppIoError;

    constexpr AppIO() noexcept = default;

private:
    inline static struct termios _orig_tty{};
    inline static bool _is_raw{false}, _restored{false};

    static void restoreTerminal() noexcept {
        if (not _is_raw or _restored) { return; }
        _restored = true;

        // Restore original terminal settings
        (void) ::tcsetattr(STDIN_FILENO, TCSANOW, &_orig_tty);

        // Clear O_NONBLOCK flag
        int flags = ::fcntl(STDIN_FILENO, F_GETFL, 0);
        if (flags != -1) {
            (void) ::fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
        }

        _is_raw = false;
    }

    static void signalHandler(int sig) noexcept {
        (void) sig;
        restoreTerminal();
        ::_exit(0);
    }

    static void setupSignalHandlers() noexcept {
        struct sigaction sa{};
        sa.sa_handler = signalHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;

        (void) ::sigaction(SIGINT, &sa, nullptr);
        (void) ::sigaction(SIGTERM, &sa, nullptr);
        (void) ::sigaction(SIGQUIT, &sa, nullptr);
    }

    KF_IMPL_INITABLE(Self, void());
    void initImpl() noexcept {
        if (::isatty(STDIN_FILENO)) {
            // Save original terminal settings
            ::tcgetattr(STDIN_FILENO, &_orig_tty);

            // Set raw mode (like Arduino UART)
            struct termios raw = _orig_tty;
            raw.c_lflag &= ~(ICANON | ECHO);
            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 0;
            ::tcsetattr(STDIN_FILENO, TCSANOW, &raw);

            // Set stdin to non-blocking mode
            int flags = ::fcntl(STDIN_FILENO, F_GETFL, 0);
            if (flags != -1) {
                (void) ::fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
            }

            _is_raw = true;
            _restored = false;

            // Register terminal restoration on normal exit
            ::atexit(restoreTerminal);

            // Install signal handlers
            setupSignalHandlers();
        }

        // Disable sync with C I/O (for raw mode)
        std::ios_base::sync_with_stdio(false);
    }

    KF_IMPL_QUITABLE(Self);
    void quitImpl() noexcept {
        restoreTerminal();
    }

    KF_IMPL_BINARY_READABLE(Self, Error);

    auto readBufferImpl(Bytes buffer) noexcept -> Result<BytesView, Error> {
        if (buffer.empty()) {
            return ok(BytesView{});
        }

        auto const n = ::read(STDIN_FILENO, buffer.data(), buffer.length());

        if (n < 0) {
            if (errno == EAGAIN or errno == EWOULDBLOCK) {
                return ok(BytesView{});
            }
            return error(Error::ReadFailed);
        }

        // EOF
        if (n == 0) {
            return error(Error::ReadFailed);
        }

        return ok(BytesView{buffer.data(), static_cast<usize>(n)});
    }

    template<trivial T> auto readPacketImpl() -> Result<T, Error> {
        T packet;

        auto result = this->readBuffer(Bytes{
            reinterpret_cast<u8 *>(&packet),
            sizeof(T),
        });

        if (result.isError()) {
            return error(result.error());
        }

        if (result.ok().length() != sizeof(T)) {
            return error(Error::ReadFailed);
        }

        return ok(packet);
    }

    KF_IMPL_READ_AVAILABLE(Self);
    usize availableForReadImpl() const noexcept {
        return std::cin.rdbuf()->in_avail();
    }

    KF_IMPL_BINARY_WRITABLE(Self, AppIoWriteResult);

    AppIoWriteResult writeBufferImpl(BytesView buffer) noexcept {
        if (not buffer.empty()) {
            auto const to_write = buffer.length();

            if (not std::cout.write(reinterpret_cast<char const *>(buffer.data()), to_write).good()) {
                return error(Error::WriteFailed);
            }
        }

        return ok();
    }

    AppIoWriteResult writePacketImpl(trivial auto const &packet) noexcept {
        constexpr auto to_write = sizeof(packet);

        if constexpr (to_write == sizeof(u8)) {
            std::cout.put(static_cast<char>(packet));
        } else {
            std::cout.write(reinterpret_cast<char const *>(&packet), to_write);
        }

        if (not std::cout.good()) {
            return error(Error::WriteFailed);
        } else {
            return ok();
        }
    }

    AppIoWriteResult writeMixedImpl(trivial auto const &header, BytesView buffer) noexcept {
        KF_TRY(this->writePacket(header));
        return this->writeBuffer(buffer);
    }

    KF_IMPL_WRITE_AVAILABLE(Self);
    usize availableForWriteImpl() const noexcept {
        return std::cout.good() ? static_cast<usize>(-1) : 0;
    }

    KF_IMPL_FLUSH(Self);
    void flushImpl() noexcept {
        (void) std::cout.flush();
    }
};

}// namespace internal

struct Init : internal::InitBase {

    /// @brief Platform-depended standard binary Input/Output Stream
    internal::AppIO io{};
};

}// namespace kf

#endif

KF_PLATFORM_APP_FUNCTION_DECLARATION {

    static kf::Init init{};

    init.io.init();

    kf::Logger::writer = [](auto str) {
        (void) init.io.writeBuffer({reinterpret_cast<kf::u8 const *>(str.data()), str.length()});
        init.io.flush();
    };

    init.logger.info("KiraFlux Toolkit v0.3.5");

    main(init);

    init.io.quit();

    return KF_PLATFORM_APP_QUIT();
}

// cleanup

#undef KF_PLATFORM_APP_FUNCTION_DECLARATION
#undef KF_PLATFORM_APP_QUIT

#undef KF_CONFIG_MAIN_ARENA_BUFFER_LENGTH
#undef KF_CONFIG_MAIN_LOGGER_KEY
#undef KF_CONFIG_MAIN_LOGGER_BUFFER_LENGTH
#undef KF_CONFIG_MAIN_UART_BAUDRATE
