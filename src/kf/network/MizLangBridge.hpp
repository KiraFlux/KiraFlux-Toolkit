// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/Function.hpp"
#include "kf/Result.hpp"
#include "kf/aliases.hpp"
#include "kf/attributes.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/io/InputStream.hpp"
#include "kf/memory/io/OutputStream.hpp"

namespace kf {

template<typename Tlc, typename Trc, usize N> struct MizLangBridge {
    using LocalCodeType = Tlc;
    using RemoteCodeType = Trc;

    enum class Error : u8 {
        UnknownInstruction,
        InstructionCodeReadFail,
        InstructionCodeWriteFail,
        InstructionSendHandlerNotReady,// добавлено
        InstructionArgumentReadFail,
        InstructionArgumentWriteFail
    };

    using SendHandler = Function<Result<void, Error>(io::OutputStream &, void *)>;
    using ReceiveHandler = Function<Result<void, Error>(io::InputStream &)>;
    using InstructionTable = Array<ReceiveHandler, N>;

private:
    InstructionTable instructions;
    io::InputStream in;
    io::OutputStream out;
    RemoteCodeType next_code{0};

public:
    explicit MizLangBridge(
        io::InputStream input_stream,
        io::OutputStream output_stream,
        InstructionTable instructions) noexcept :
        in{std::move(input_stream)},
        out{std::move(output_stream)},
        instructions{std::move(instructions)} {}

    struct Instruction {
    private:
        io::OutputStream &out;
        const SendHandler handler;
        const RemoteCodeType code;

    public:
        Instruction(io::OutputStream &output_stream, RemoteCodeType code, SendHandler call_handler) noexcept :
            out{output_stream}, handler{std::move(call_handler)}, code{code} {}

        Instruction(Instruction &&other) noexcept :
            out{other.out}, handler{std::move(other.handler)}, code{other.code} {}

        kf_nodiscard Result<void, Error> send(void *args) noexcept {
            if (not handler) {
                return {Error::InstructionSendHandlerNotReady};
            }
            if (not out.write(code)) {
                return {Error::InstructionCodeWriteFail};
            }
            return handler(out, args);
        }

        Instruction() = delete;
    };

    kf_nodiscard Instruction createInstruction(SendHandler handler) noexcept {
        return Instruction{out, next_code++, std::move(handler)};
    }

    kf_nodiscard Result<void, Error> poll() noexcept {
        if (in.available() < sizeof(LocalCodeType)) { return {}; }

        auto code_option = in.read<LocalCodeType>();

        if (not code_option.hasValue()) {
            return {Error::InstructionCodeReadFail};
        }

        const auto code = code_option.value();
        if (code >= instructions.size()) {
            in.clean();
            return {Error::UnknownInstruction};
        }

        return instructions[code](in);
    }
};

}// namespace kf