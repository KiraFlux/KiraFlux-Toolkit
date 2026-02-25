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

/// @brief Structural Serial P2P byte-packet protocol
/// @tparam N Exact count of local instructions
/// @tparam Tlc Type of local instruction code
/// @tparam Trc Type of remote instruction code
template<usize N, typename Tlc, typename Trc = Tlc> struct MizLangBridge {
    enum class Error : u8 {
        Receiver_CodeNotExists,
        Receiver_CodeReadFail,
        Receiver_ArgumentReadFail,

        Sender_CodeWriteFail,
        Sender_FunctionNotReady,
        Sender_ArgumentWriteFail
    };

    using LocalCodeType = Tlc;
    using RemoteCodeType = Trc;
    using SendFunctionType = Function<Result<void, Error>(io::OutputStream &, void *)>;
    using ReceiveFunctionType = Function<Result<void, Error>(io::InputStream &)>;
    using InstructionTableType = Array<ReceiveFunctionType, N>;

private:
    InstructionTableType instructions;
    io::InputStream in;
    io::OutputStream out;
    RemoteCodeType next_code{0};

public:
    explicit MizLangBridge(
        io::InputStream input_stream,
        io::OutputStream output_stream,
        InstructionTableType instructions) noexcept :
        in{std::move(input_stream)},
        out{std::move(output_stream)},
        instructions{std::move(instructions)} {}

    struct Instruction {
    private:
        const SendFunctionType sender;
        io::OutputStream &out;
        const RemoteCodeType code;

    public:
        explicit Instruction(io::OutputStream &output_stream, RemoteCodeType code, SendFunctionType sender) noexcept :
            out{output_stream}, sender{std::move(sender)}, code{code} {}

        Instruction(Instruction &&other) noexcept :
            out{other.out}, sender{std::move(other.sender)}, code{other.code} {}

        [[nodiscard]] Result<void, Error> send(void *args) noexcept {
            if (not sender) {
                return {Error::Sender_FunctionNotReady};
            }
            if (not out.write(code)) {
                return {Error::Sender_CodeWriteFail};
            }
            return sender(out, args);
        }
    };

    [[nodiscard]] Instruction createInstruction(SendFunctionType sender_function) noexcept {
        return Instruction{out, next_code++, std::move(sender_function)};
    }

    [[nodiscard]] Result<void, Error> poll() noexcept {
        if (in.available() < sizeof(LocalCodeType)) { return {}; }

        auto code_option = in.read<LocalCodeType>();

        if (not code_option.hasValue()) {
            return {Error::Receiver_CodeReadFail};
        }

        const auto code = code_option.value();
        if (code >= instructions.size()) {
            in.clean();
            return {Error::Receiver_CodeNotExists};
        }

        return instructions[code](in);
    }
};

}// namespace kf