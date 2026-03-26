// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/Function.hpp"
#include "kf/Result.hpp"
#include "kf/aliases.hpp"
#include "kf/io/Readable.hpp"
#include "kf/io/Writable.hpp"
#include "kf/memory/Slice.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf::network {

/// @brief Structural Serial P2P byte-packet protocol
/// @tparam Tlc Type of local instruction code
/// @tparam Trc Type of remote instruction code
template<typename R, typename W, typename Tlc, typename Trc = Tlc> struct MizLangBridge final : mixin::NonCopyable {
    KF_CHECK_IMPL(R, ::kf::io::ReadableTag);
    KF_CHECK_IMPL(W, ::kf::io::WritableTag);

    using ReadableImpl = R;
    using WritableImpl = W;
    using LocalCodeType = Tlc;
    using RemoteCodeType = Trc;

    enum class Error : u8 {
        Receiver_CodeNotExists,
        Receiver_CodeReadFail,
        Receiver_ArgumentReadFail,

        Sender_CodeWriteFail,
        Sender_FunctionNotReady,
        Sender_ArgumentWriteFail
    };

    using SendFunctionType = Function<Result<void, Error>(WritableImpl &, void *)>;
    using ReceiveFunctionType = Function<Result<void, Error>(ReadableImpl &)>;
    using InstructionTableType = memory::Slice<ReceiveFunctionType>;

    explicit MizLangBridge(ReadableImpl &&input_stream, WritableImpl &&output_stream, InstructionTableType instructions) noexcept :
        _input_stream{std::move(input_stream)}, _output_stream{std::move(output_stream)}, _instructions{instructions} {}

    struct Instruction final : mixin::NonCopyable {

        explicit Instruction(WritableImpl &output_stream, RemoteCodeType code, SendFunctionType &&sender) noexcept :
            _output_stream{output_stream}, _sender{std::move(sender)}, _code{code} {}

        Instruction(Instruction &&other) noexcept :
            _output_stream{other._output_stream}, _sender{std::move(other._sender)}, _code{other._code} {}

        [[nodiscard]] Result<void, Error> send(void *args) noexcept {
            if (not _sender) { return {Error::Sender_FunctionNotReady}; }
            if (_output_stream.writePacket(_code).isError()) { return {Error::Sender_CodeWriteFail}; }
            return _sender(_output_stream, args);
        }

    private:
        const SendFunctionType _sender;
        WritableImpl &_output_stream;
        const RemoteCodeType _code;
    };

    [[nodiscard]] Instruction createInstruction(SendFunctionType &&sender_function) noexcept {
        return Instruction{_output_stream, _next_code++, std::move(sender_function)};
    }

    /// @brief Poll for incoming instructions and process them.
    /// @return Result<void, Error>:
    ///   - Error if reading the instruction code fails, the code is unknown, or argument reading fails.
    [[nodiscard]] Result<void, Error> poll() noexcept {
        const auto code_result = _input_stream.template readPacket<LocalCodeType>();
        if (not code_result.isError()) { return {Error::Receiver_CodeReadFail}; }

        const auto code = code_result.value();
        if (code >= _instructions.size()) { return {Error::Receiver_CodeNotExists}; }

        return _instructions[code](_input_stream);
    }

private:
    InstructionTableType _instructions;
    ReadableImpl _input_stream;
    WritableImpl _output_stream;
    RemoteCodeType _next_code{0};
};

}// namespace kf::network