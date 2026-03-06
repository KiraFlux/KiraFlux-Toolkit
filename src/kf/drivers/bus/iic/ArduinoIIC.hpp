// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Wire.h>

#include "kf/aliases.hpp"
#include "kf/memory/io/Readable.hpp"
#include "kf/memory/io/Writable.hpp"

#include "kf/drivers/bus/iic/IIC.hpp"

namespace kf::drivers::bus::iic {

struct ArduinoIIC : IIC<ArduinoIIC> {

    enum class Error {
        BusBusy,
        AddressNack,
        DataNack,
        Timeout,
        Unknown,
    };

    struct Config {
        u32 clock;
    };

    struct Node : memory::io::Readable<Node>, memory::io::Writable<Node> {
        struct Config {
            u8 address;
        };

        explicit Node(ArduinoIIC &bus, const Config &config) noexcept : _bus{bus}, _config{config} {}

    private:
        const Config &_config;
        ArduinoIIC &_bus;

        // Readable impl
        friend Readable<Node>;

        [[nodiscard]] Result<u8, ErrorImpl> readByteimpl() noexcept {
            if (_bus._wire.requestFrom(_config.address, sizeof(u8)) == 0) { return {Error::Timeout}; }

            const u8 byte = _bus._wire.read();

            return {byte};
        }

        [[nodiscard]] Result<memory::Slice<u8>, Error> readBuffer(void *source, usize size) {
            const usize received = _bus._wire.requestFrom(_config.address, static_cast<u8>(size));

            if (received == 0) { return {Error::Timeout}; }

            (void) _bus._wire.readBytes(static_cast<u8 *>(source), received);

            return {memory::Slice<u8>{static_cast<u8 *>(source), received}};
        }
        //

        // Writable impl
        friend Writable<Node>;
    };

    explicit ArduinoIIC(const Config &config, TwoWire &wire) noexcept : _config{config}, _wire{wire} {}

private:
    const Config &_config;
    TwoWire &_wire;

    // IIC impl
    friend IIC<ArduinoIIC>;

    [[nodiscard]] Result<void, Error> initImpl() noexcept {
        (void) _wire.begin();

        return {};
    }

    void quitImpl() noexcept {
        (void) _wire.end();
    }
};

}// namespace kf::drivers::bus::iic
