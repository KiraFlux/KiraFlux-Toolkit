// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/Function.hpp"
#include "kf/Result.hpp"
#include "kf/aliases.hpp"
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
    using SendFunctionType = Function<Result<void, Error>(memory::io::OutputStream &, void *)>;
    using ReceiveFunctionType = Function<Result<void, Error>(memory::io::InputStream &)>;
    using InstructionTableType = Array<ReceiveFunctionType, N>;

private:
    InstructionTableType _instructions;
    memory::io::InputStream _input_stream;
    memory::io::OutputStream _output_stream;
    RemoteCodeType _next_code{0};

public:
    explicit MizLangBridge(
        memory::io::InputStream input_stream,
        memory::io::OutputStream output_stream,
        InstructionTableType instructions) noexcept :
        _input_stream{std::move(input_stream)},
        _output_stream{std::move(output_stream)},
        _instructions{std::move(instructions)} {}

    struct Instruction {
    private:
        const SendFunctionType _sender;
        memory::io::OutputStream &_output_stream;
        const RemoteCodeType _code;

    public:
        explicit Instruction(memory::io::OutputStream &output_stream, RemoteCodeType code, SendFunctionType sender) noexcept :
            _output_stream{output_stream}, _sender{std::move(sender)}, _code{code} {}

        Instruction(Instruction &&other) noexcept :
            _output_stream{other._output_stream}, _sender{std::move(other._sender)}, _code{other._code} {}

        [[nodiscard]] Result<void, Error> send(void *args) noexcept {
            if (not _sender) {
                return {Error::Sender_FunctionNotReady};
            }
            if (not _output_stream.write(_code)) {
                return {Error::Sender_CodeWriteFail};
            }
            return _sender(_output_stream, args);
        }
    };

    [[nodiscard]] Instruction createInstruction(SendFunctionType sender_function) noexcept {
        return Instruction{_output_stream, _next_code++, std::move(sender_function)};
    }

    /// @brief Poll for incoming instructions and process them.
    /// @return Result<void, Error>:
    ///   - Success (empty result) if no complete instruction is available (insufficient data).
    ///   - Error if reading the instruction code fails, the code is unknown, or argument reading fails.
    /// @note This method does not block. If there isn't enough data to read the instruction code,
    ///       it returns success without processing anything.
    [[nodiscard]] Result<void, Error> poll() noexcept {
        if (_input_stream.available() < sizeof(LocalCodeType)) { return {}; }

        const auto code_option = _input_stream.read<LocalCodeType>();

        if (not code_option.hasValue()) {
            return {Error::Receiver_CodeReadFail};
        }

        const auto code = code_option.value();
        if (code >= _instructions.size()) {
            _input_stream.clean();
            return {Error::Receiver_CodeNotExists};
        }

        return _instructions[code](_input_stream);
    }
};

}// namespace kf