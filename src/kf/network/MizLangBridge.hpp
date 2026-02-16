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

/// @brief MizLang structural packet P2P Protocol
/// @tparam Tlc Local instruction code type
/// @tparam Trc Remote instruction code type
/// @tparam N Local instruction table len
template<typename Tlc, typename Trc, usize N> struct MizLangBridge {
    using LocalCodeType = Tlc;
    using RemoteCodeType = Trc;


    /// @brief Instruction processing errors
    /// @note Used for error handling in instruction-based communication protocol
    enum class Error : u8 {
        UnknownInstruction,          ///< Received instruction code doesn't match any known instruction
        InstructionCodeReadFail,     ///< Failed to read instruction code from stream
        InstructionCodeWriteFail,    ///< Failed to write instruction code to stream
        InstructionSendHandlerIsNull,///< Instruction send handler not set (nullptr)
        InstructionArgumentReadFail, ///< Failed to read instruction argument (for user instructions)
        InstructionArgumentWriteFail ///< Failed to write instruction argument (for user instructions)
    };


    /// @brief Handler type for sending instruction arguments
    using ReceiveHandler = Function<Result<void, Error>(io::OutputStream &, Args...)>;

    using SendHandler = Function<Result<void, Error>(io::InputStream &)>;

private:
    Array<SendHandler, N> instructions;///< Table of instruction handlers indexed by code
    io::InputStream in;
    io::OutputStream out;
    RemoteCodeType next_code{0};///< Next available instruction code (auto-incremented)

public:
    explicit MizLangBridge(
        io::InputStream &&input_stream,
        io::OutputStream &&output_stream,
        std::initializer_list<ReceiveHandler> instructions) noexcept :
        in{input_stream}, out{output_stream}, instructions{instructions} {}

    /// @brief Send instruction wrapper for serializing and transmitting commands
    /// @tparam Args Types of arguments to send with instruction
    /// @note Encapsulates instruction code and handler for sending arguments
    template<typename... Args> struct Instruction {
    private:
        io::OutputStream &out;    ///< Output stream for writing data
        const Handler handler;    ///< Handler function for argument serialization
        const RemoteCodeType code;///< Instruction code to send

    public:
        /// @brief Construct instruction with stream, code and handler
        /// @param output_stream Output stream for writing
        /// @param code Instruction code to identify this instruction
        /// @param call_handler Handler function for argument serialization
        Instruction(io::OutputStream &output_stream, RemoteCodeType code, Handler call_handler) noexcept :
            out{output_stream}, handler{std::move(call_handler)}, code{code} {}

        Instruction(Instruction &&other) noexcept :
            out{other.out}, handler{std::move(other.handler)}, code{other.code} {}

        /// @brief Execute instruction with given arguments
        /// @param args Arguments to pass to handler for serialization
        /// @return Result indicating success or specific error
        /// @note Writes instruction code then calls handler for argument serialization
        kf_nodiscard Result<void, Error> send(Args... args) noexcept {
            if (not handler) {
                return {Error::InstructionSendHandlerNotReady};
            }

            if (not out.write(code)) {
                return {Error::InstructionCodeWriteFail};
            }

            return handler(out, args...);
        }

        /// @brief Deleted default constructor
        Instruction() = delete;
    };

    /// @brief Create new send instruction with auto-assigned code
    /// @tparam Args Types of arguments for the instruction
    /// @param handler Function to serialize arguments to output stream
    /// @return Instruction object ready to be called with arguments
    /// @note Automatically assigns next available instruction code
    template<typename... Args> kf_nodiscard Instruction<RemoteCodeType, Args...> createInstruction(
        typename Instruction<RemoteCodeType, Args...>::Handler handler) noexcept {
        return Instruction<RemoteCodeType, Args...>{out, next_code++, std::move(handler)};
    }

    /// @brief Poll for incoming instructions and process them
    /// @return Result indicating success or specific error
    /// @note Checks for available data and dispatches to appropriate handler
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