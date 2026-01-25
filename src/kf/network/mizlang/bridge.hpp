// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"
#include "kf/Result.hpp"
#include "kf/core/utility.hpp"
#include "kf/memory/Array.hpp"
#include "kf/network/mizlang/streams.hpp"

namespace kf::mizlang::bridge {

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

/// @brief Instruction receiver for handling incoming commands
/// @tparam T Instruction code type (typically enum or integer)
/// @tparam N Maximum number of distinct instructions supported
/// @note Maps instruction codes to handler functions for processing incoming data
template<typename T, usize N> struct Receiver {
    using Code = T;                             ///< Instruction code type for incoming instructions
    static constexpr auto instruction_count = N;///< Maximum number of supported instructions

    /// @brief Instruction handler table type
    /// @note Array of functions that process instructions based on their code
    using InstructionTable = Array<Function<Result<void, Error>(InputStream &)>, instruction_count>;

    InputStream in;               ///< Input stream for reading incoming data
    InstructionTable instructions;///< Table of instruction handlers indexed by code

    /// @brief Poll for incoming instructions and process them
    /// @return Result indicating success or specific error
    /// @note Checks for available data and dispatches to appropriate handler
    Result<void, Error> poll() noexcept {
        if (in.available() < sizeof(Code)) { return {}; }

        auto code_option = in.read<Code>();

        if (not code_option.hasValue()) {
            return {Error::InstructionCodeReadFail};
        }

        const auto code = code_option.value();

        if (code >= instruction_count) {
            in.clean();
            return {Error::UnknownInstruction};
        }

        return instructions[code](in);
    }

    /// @brief Deleted default constructor (must provide stream and handlers)
    Receiver() = delete;
};

/// @brief Send instruction wrapper for serializing and transmitting commands
/// @tparam T Instruction code type
/// @tparam Args Types of arguments to send with instruction
/// @note Encapsulates instruction code and handler for sending arguments
template<typename T, typename... Args> struct Instruction {
    using Code = T;///< Instruction code type for sending

    /// @brief Handler type for sending instruction arguments
    using Handler = Function<Result<void, Error>(OutputStream &, Args...)>;

private:
    OutputStream &out;    ///< Output stream for writing data
    const Handler handler;///< Handler function for argument serialization
    const Code code;      ///< Instruction code to send

public:
    /// @brief Construct instruction with stream, code and handler
    /// @param output_stream Output stream for writing
    /// @param code Instruction code to identify this instruction
    /// @param call_handler Handler function for argument serialization
    Instruction(OutputStream &output_stream, Code code, Handler call_handler) noexcept:
        out{output_stream}, handler{kf::move(call_handler)}, code{code} {}

    /// @brief Move constructor
    /// @param other Instruction to move from
    Instruction(Instruction &&other) noexcept:
        out{other.out}, handler{kf::move(other.handler)}, code{other.code} {}

    /// @brief Execute instruction with given arguments
    /// @param args Arguments to pass to handler for serialization
    /// @return Result indicating success or specific error
    /// @note Writes instruction code then calls handler for argument serialization
    Result<void, Error> operator()(Args... args) noexcept {
        if (nullptr == handler) {
            return {Error::InstructionSendHandlerIsNull};
        }

        if (not out.write(code)) {
            return {Error::InstructionCodeWriteFail};
        }

        return handler(out, args...);
    }

    /// @brief Deleted default constructor
    Instruction() = delete;
};

/// @brief Protocol sender for creating and managing send instructions
/// @tparam T Instruction code type for sending
/// @note Manages instruction code assignment and creates Instruction objects
template<typename T> struct Sender {
    using Code = T;///< Instruction code type for sending

private:
    OutputStream out; ///< Output stream for writing
    Code next_code{0};///< Next available instruction code (auto-incremented)

public:
    /// @brief Construct sender with output stream
    /// @param output_stream Output stream for writing instructions
    explicit Sender(OutputStream &&output_stream) noexcept:
        out{output_stream} {}

    /// @brief Create new send instruction with auto-assigned code
    /// @tparam Args Types of arguments for the instruction
    /// @param handler Function to serialize arguments to output stream
    /// @return Instruction object ready to be called with arguments
    /// @note Automatically assigns next available instruction code
    template<typename... Args> Instruction<Code, Args...> createInstruction(typename Instruction<Code, Args...>::Handler handler) noexcept{
        return Instruction<Code, Args...>{out, next_code++, kf::move(handler)};
    }
};

}// namespace kf::mizlang::bridge