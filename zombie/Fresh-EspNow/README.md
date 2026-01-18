# Fresh EspNow

**Современная C++ обёртка для ESP-NOW**

## Установка

```ini
lib_deps = https://github.com/KiraFlux/Fresh-EspNow.git
```

## API

### Основные типы

```cpp
// Инкапсуляция протокола
kf::EspNow

// MAC Адрес
kf::EspNow::Mac = std::array<kf::u8, 6>  

// Обработчик приёма от неизвестного пира
kf::EspNow::UnknownReceiveHandler = std::function<const Mac&, const kf::slice<const void>>

// Управление пиром
kf::EspNow::Peer

// Обработчик приёма
kf::EspNow::Peer::ReceiveHandler = std::function<const kf::slice<const void>>
```

### Инициализация

```cpp

kf::EspNow {

// Инициализация ESP-NOW протокола
init() -> kf::Result<void, kf::EspNow::Error>

// Деинициализация ESP-NOW протокола
quit() -> void

// Собственный MAC-адрес устройства
mac() -> const kf::EspNow::Mac&

}
```

### Управление пирами

```cpp
kf::EspNow::Peer {

// Добавление пира в систему ESP-NOW
add(const kf::EspNow::Mac& mac) -> kf::Result<kf::EspNow::Peer, kf::EspNow::Error>

// Удаление пира из системы ESP-NOW
del() -> kf::Result<void, kf::EspNow::Error>

// Проверка существования пира в системе
exist() -> bool

}
```

### Передача данных

```cpp
kf::EspNow::Peer {

// Отправка структуры данных (проверка размера на этапе компиляции)
sendPacket(const T& value) -> kf::Result<void, kf::EspNow::Error>

// Отправка бинарного буфера (проверка размера в runtime)
sendBuffer(kf::slice<const void> buffer) -> kf::Result<void, kf::EspNow::Error>

}
```

### Обработчики приёма

```cpp
// Установка обработчика входящих сообщений от пира
kf::EspNow::Peer::setReceiveHandler(Peer::ReceiveHandler&& handler) -> kf::Result<void, kf::EspNow::Error>

// Установка обработчика сообщений от неизвестных пиров
kf::EspNow::setUnknownReceiveHandler(EspNow::UnknownReceiveHandler&& handler) -> void
```

### Утилиты

```cpp
kf::EspNow {

// Конвертация MAC-адреса в строковое представление
stringFromMac(const kf::EspNow::Mac& mac) -> kf::ArrayString<mac_string_size>

// Конвертация ошибки в строковое представление
stringFromError(kf::EspNow::Error error) -> const char*

}
```

## Warning

На данный момент не является потокобезопасным

## TODOs

- Пир будет инкапсулировать stream, таким образом будет убран callback из другой задачи в пользовательском коде.

## Лицензия

MIT License - Подробнее в [LICENSE](LICENSE)