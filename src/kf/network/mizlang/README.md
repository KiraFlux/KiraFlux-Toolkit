# ByteLang Bridge

Структурированный мост для embedded-коммуникации

## Установка

**PlatformIO:**
```ini
lib_deps = 
    https://github.com/KiraFlux/ByteLang-Bridge.git
```

## Пример использования

```cpp
#include <Arduino.h>
#include <bytelang/bridge.hpp>

using namespace bytelang::bridge;

struct RobotBridge {
    // Приёмник: 2 команды, код команды - uint8_t
    Receiver<rs::u8, 2> receiver{
        .in = core::InputStream{Serial},
        .handlers = {
            // Команда 0x00: set_led(state)
            [](core::InputStream& in) -> rs::Result<void, Error> {
                auto state = in.read<rs::u8>();
                if (state.none()) return Error::InstructionArgumentReadFail;
                digitalWrite(LED_BUILTIN, state.value());
                return {};
            },
            // Команда 0x01: set_motors(left, right)
            [](core::InputStream& in) -> rs::Result<void, Error> {
                auto left = in.read<rs::u8>();
                auto right = in.read<rs::u8>();
                if (left.none() || right.none()) return Error::InstructionArgumentReadFail;
                analogWrite(9, left.value());
                analogWrite(10, right.value());
                return {};
            }
        }
    };
    
    // Отправитель данных
    Sender<rs::u8> sender{core::OutputStream{Serial}};
    
    // Инструкция для отправки данных
    auto send_sensors = sender.createInstruction(
        [](core::OutputStream& out) -> rs::Result<void, Error> {
            if (!out.write(rs::u16(analogRead(A0)))) return Error::InstructionArgumentWriteFail;
            return {};
        }
    );
};

RobotBridge bridge;

void setup() {
    Serial.begin(115200);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    bridge.receiver.poll();
    delay(1);
}
```

## API

### Receiver - приём команд
```cpp
// CodeType - тип кода команды (uint8_t, uint16_t)
// count - количество поддерживаемых команд
Receiver<CodeType, count> receiver{
    .in = core::InputStream{stream},  // входной поток
    .handlers = {handler1, handler2, ...}  // обработчики команд
};

// Опрос новых команд (вызывать в loop)
auto result = receiver.poll();
if (result.fail()) {
    // обработка ошибки
}
```

### Sender + Instruction - отправка данных
```cpp
Sender<CodeType> sender{core::OutputStream{stream}};

// Создание инструкции отправки
auto instruction = sender.createInstruction([](core::OutputStream& out) {
    // сериализация данных
    if (!out.write(data)) return Error::InstructionArgumentWriteFail;
    return rs::Result<void, Error>{};
});

// Отправка инструкции
instruction();
```

### Потоки ввода/вывода
- `core::InputStream` - чтение данных: `read<T>()`, `readByte()`, `available()`
- `core::OutputStream` - запись данных: `write(T)`, `writeByte()`

## Коды ошибок

| Ошибка                         | Значение                          |
| ------------------------------ | --------------------------------- |
| `UnknownInstruction`           | Неизвестный код команды           |
| `InstructionCodeReadFail`      | Ошибка чтения кода команды        |
| `InstructionCodeWriteFail`     | Ошибка записи кода команды        |
| `InstructionSendHandlerIsNull` | Обработчик отправки не установлен |
| `InstructionArgumentReadFail`  | Ошибка чтения аргумента команды   |
| `InstructionArgumentWriteFail` | Ошибка записи аргумента команды   |

## Лицензия
MIT License