#pragma once

#include <array>
#include <functional>
#include <kf/Result.hpp>

#include "bytelang/core/streams.hpp"

namespace bytelang::bridge {

/// Ошибки обработки инструкций
enum class Error : kf::u8 {

    /// Полученный код инструкции не соответствует ни одной инструкции.
    UnknownInstruction,

    /// Не удалось считать код инструкции
    InstructionCodeReadFail,

    /// Не удалось отправить код инструкции
    InstructionCodeWriteFail,

    /// Обработчик исполнения инструкции отправки не указан
    InstructionSendHandlerIsNull,

    // для применения в пользовательских инструкциях

    /// Не удалось прочесть аргумент
    InstructionArgumentReadFail,

    /// Не удалось записать аргумент
    InstructionArgumentWriteFail
};

/// Слушатель инструкций (Принимает код и аргументы)
template<

    /// Тип кода принимаемой инструкции
    typename T,

    /// Количество инструкций
    kf::usize N
    //
    >
struct Receiver {

    /// Код инструкции на приём
    using Code = T;

    /// Количество инструкций
    static constexpr auto instruction_count = N;

    /// Контейнер для таблицы инструкций
    using InstructionTable = std::array<std::function<kf::Result<void, Error>(core::InputStream &)>, instruction_count>;

    /// Сериализатор
    core::InputStream in;

    /// Обработчики на приём
    InstructionTable instructions;

    /// Обновление (Пул проверки)
    kf::Result<void, Error> poll() {
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

    // Запрет неявного создания
    Receiver() = delete;
};

/// Инструкция отправки
template<

    /// Примитив кода инструкции
    typename T,

    /// Сигнатура аргументов
    typename... Args
    //
    >
struct Instruction {

    /// Код инструкции на отправку
    using Code = T;

    /// Обработчик вызова инструкции
    using Handler = std::function<kf::Result<void, Error>(core::OutputStream &, Args...)>;

private:
    /// Сериализатор
    core::OutputStream &out;

    /// Обработчик вызова
    const Handler handler;

    /// Код инструкции
    const Code code;

public:
    Instruction(core::OutputStream &output_stream, Code code, Handler call_handler) :
        out{output_stream}, handler{std::move(call_handler)}, code{code} {}

    Instruction(Instruction &&other) noexcept :
        out{other.out}, handler{std::move(other.handler)}, code{other.code} {}

    /// Вызвать инструкцию
    kf::Result<void, Error> operator()(Args... args) {
        if (nullptr == handler) {
            return {Error::InstructionSendHandlerIsNull};
        }

        if (not out.write(code)) {
            return {Error::InstructionCodeWriteFail};
        }

        return handler(out, args...);
    }

    Instruction() = delete;
};

/// Протокол отправки
template<

    /// Тип кода отправляемой инструкции
    typename T
    //
    >
struct Sender {

    /// Тип кода отправляемой инструкции
    using Code = T;

private:
    /// Сериализатор
    core::OutputStream out;

    /// Счётчик инструкций
    Code next_code{0};

public:
    explicit Sender(core::OutputStream &&output_stream) :
        out{output_stream} {}

    /// Создать инструкцию отправки
    template<typename... Args> Instruction<Code, Args...> createInstruction(typename Instruction<Code, Args...>::Handler handler) {
        return Instruction<Code, Args...>{out, next_code++, std::move(handler)};
    }
};

}// namespace bytelang::bridge
