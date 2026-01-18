#pragma once

#include <array>
#include <map>
#include <functional>

#include <esp_now.h>
#include <esp_mac.h>

#include <WiFi.h>

#include <kf/aliases.hpp>
#include <kf/slice.hpp>
#include <kf/Result.hpp>
#include <kf/String.hpp>
#include <kf/tools/meta/Singleton.hpp>


namespace kf {

/// @brief Инкапсулирует работу ESP NOW в безопасных абстракциях
struct EspNow : tools::Singleton<EspNow> {
    friend struct Singleton<EspNow>;

    /// @brief Безопасный тип для MAC адреса
    using Mac = std::array<u8, ESP_NOW_ETH_ALEN>;

    /// @brief Тип обработчика приёма от неизвестного пира
    using UnknownReceiveHandler = std::function<void(const Mac &, const slice<const void>)>;

    /// @brief Перечисление ошибок операций API Espnow
    enum class Error : u8 {

        // Инициализация

        /// @brief Внутренняя ошибка ESP-NOW API
        InternalError,

        /// @brief Неизвестная ошибка ESP API
        UnknownError,

        /// @brief Протокол ESP-NOW не был инициализирован
        NotInitialized,

        /// @brief Установлен неверный режим интерфейса WiFi
        IncorrectWiFiMode,

        // Работа с пирами

        /// @brief Список пиров полон
        PeerListIsFull,

        /// @brief Неверный аргумент
        InvalidArg,

        /// @brief Не хватает памяти для добавления пира
        NoMemory,

        /// @brief Пир уже добавлен
        PeerAlreadyExists,

        /// @brief Пир не найден в списке добавленных
        PeerNotFound,

        // Другое

        /// @brief Слишком большое сообщение
        TooBigMessage,
    };

    /// @brief Пир
    struct Peer {

        /// @brief Обработчик приёма
        using ReceiveHandler = std::function<void(const slice<const void> &)>;

        /// @brief Контекст пира
        struct Context {

            /// @brief Обработчик на приём
            ReceiveHandler on_receive{nullptr};
        };

    private:
        Mac mac_;

    public:

        /// @brief Добавить пир
        static Result<Peer, Error> add(const Mac &mac) {
            esp_now_peer_info_t peer = {
                .channel = 0,
                .ifidx = WIFI_IF_STA,
                .encrypt = false,
            };

            std::copy(mac.begin(), mac.end(), peer.peer_addr);

            const auto result = esp_now_add_peer(&peer); // guru

            if (ESP_OK == result) {
                return {Peer{mac}};
            } else {
                return {translateEspnowError(result)};
            }
        }

        /// @brief MAC адрес пира
        [[nodiscard]] const Mac &mac() const { return mac_; }

        /// @brief Отправить пакет
        template<typename T> [[nodiscard]] Result<void, Error> sendPacket(const T &value) {
            static_assert(sizeof(T) < ESP_NOW_MAX_DATA_LEN, "Message is too big!");
            return processSend(static_cast<const void *>(&value), sizeof(T));
        }

        /// @brief Отправить буфер
        [[nodiscard]] Result<void, Error> sendBuffer(slice<const void> buffer) {
            if (buffer.size() > ESP_NOW_MAX_DATA_LEN) {
                return {Error::TooBigMessage};
            }

            return processSend(buffer.data(), buffer.size());
        }

        /// Установить обработчик входящих сообщений
        Result<void, Error> setReceiveHandler(ReceiveHandler &&handler) {
            if (not exist()) {
                // peer not exist - cannot set a handler
                return {Error::PeerNotFound};
            }

            auto &espnow = EspNow::instance();
            auto context = espnow.getPeerContext(mac_);

            if (nullptr == context) {
                // context not exist yet - create and insert
                espnow.peer_contexts.insert({mac_, Context{std::move(handler)}});
            } else {
                // context already exist. Just mutate
                context->on_receive = std::move(handler);
            }

            return {};
        }

        /// @brief Удалить пир
        [[nodiscard]] Result<void, Error> del() {
            auto &espnow = EspNow::instance();

            if (nullptr != espnow.getPeerContext(mac_)) {
                // context exist - delete it
                espnow.peer_contexts.erase(mac_);
            }

            const auto result = esp_now_del_peer(mac_.data());

            if (ESP_OK == result) {
                return {};
            } else {
                return {translateEspnowError(result)};
            }
        }

        /// @brief Проверить существование пира
        [[nodiscard]] bool exist() {
            return esp_now_is_peer_exist(mac_.data());
        }

    private:

        [[nodiscard]] Result<void, Error> processSend(const void *data, usize len) {
            const auto result = esp_now_send(
                mac_.data(),
                static_cast<const u8 *>(data),
                len
            );

            if (ESP_OK == result) {
                return {};
            } else {
                return {translateEspnowError(result)};
            }
        }

        // Создание пира только через Peer::add
        explicit Peer(const Mac &mac) :
            mac_{mac} {}
    };

private:
    std::map<Mac, Peer::Context> peer_contexts{};

    UnknownReceiveHandler unknown_receive_handler{nullptr};

    const Mac mac_{
        []() -> Mac {
            Mac ret{};
            esp_read_mac(ret.data(), ESP_MAC_WIFI_STA);
            return ret;
        }()
    };

public:

    /// @brief Инициализировать протокол ESP-NOW
    [[nodiscard]] static Result<void, Error> init() {
        const auto wifi_ok = WiFiClass::mode(WIFI_MODE_STA);
        if (not wifi_ok) {
            return {Error::InternalError};
        }

        const auto init_result = esp_now_init();
        if (ESP_OK != init_result) {
            return {translateEspnowError(init_result)};
        }

        const auto handler_result = esp_now_register_recv_cb(onReceive);
        if (ESP_OK != handler_result) {
            return {translateEspnowError(handler_result)};
        }

        return {};
    }

    /// @brief Завершить работу протокола
    static void quit() {
        (void) esp_now_unregister_recv_cb();

        (void) esp_now_deinit();
    }

    /// @brief Собственный адрес
    [[nodiscard]] const Mac &mac() const { return mac_; }

    /// @brief Установить обработчик приёма сообщения от неизвестного мира
    void setUnknownReceiveHandler(UnknownReceiveHandler &&handler) {
        unknown_receive_handler = std::move(handler);
    }

private:

    static void onReceive(const u8 *raw_mac_address, const u8 *data, int size) {
        auto &self = EspNow::instance();

        const auto &source_address = *reinterpret_cast<const Mac *>(raw_mac_address);
        const slice<const void> buffer{data, static_cast<usize>(size)};

        const auto peer_context = self.getPeerContext(source_address);

        if (nullptr == peer_context) {
            // unknown peer
            if (nullptr == self.unknown_receive_handler) { return; }

            self.unknown_receive_handler(source_address, buffer);

            return;
        } else {
            // known peer
            if (nullptr == peer_context->on_receive) { return; }

            peer_context->on_receive(buffer);
        }
    }

    Peer::Context *getPeerContext(const Mac &peer_mac) {
        auto it = peer_contexts.find(peer_mac);

        if (it == peer_contexts.end()) {
            return nullptr;
        } else {
            return &it->second;
        }
    };

    /// @brief Перевод результата esp error в значение ошибки
    static Error translateEspnowError(esp_err_t result) {
        switch (result) {
            case ESP_ERR_ESPNOW_INTERNAL: return Error::InternalError;
            case ESP_ERR_ESPNOW_NOT_INIT: return Error::NotInitialized;
            case ESP_ERR_ESPNOW_ARG: return Error::InvalidArg;
            case ESP_ERR_ESPNOW_NO_MEM: return Error::NoMemory;
            case ESP_ERR_ESPNOW_NOT_FOUND: return Error::PeerNotFound;
            case ESP_ERR_ESPNOW_IF: return Error::IncorrectWiFiMode;
            case ESP_ERR_ESPNOW_FULL: return Error::PeerListIsFull;
            case ESP_ERR_ESPNOW_EXIST: return Error::PeerAlreadyExists;
            default: return Error::UnknownError;
        }
    }

    static constexpr auto mac_string_size = sizeof("0000-0000-0000");

public:

    /// @brief Преобразовать MAC адрес в массив-строку
    static ArrayString<mac_string_size> stringFromMac(const Mac &mac) {
        ArrayString<mac_string_size> ret{};
        const auto p = mac.data();
        formatTo(ret, "%02x%02x-%02x%02x-%02x%02x", p[0], p[1], p[2], p[3], p[4], p[5]);
        return ret;
    }

#define return_case(__v) case __v: return #__v

    static const char *stringFromError(kf::EspNow::Error error) {
        switch (error) {
            return_case(kf::EspNow::Error::NotInitialized);
            return_case(kf::EspNow::Error::InternalError);
            return_case(kf::EspNow::Error::TooBigMessage);
            return_case(kf::EspNow::Error::InvalidArg);
            return_case(kf::EspNow::Error::NoMemory);
            return_case(kf::EspNow::Error::PeerNotFound);
            return_case(kf::EspNow::Error::IncorrectWiFiMode);
            return_case(kf::EspNow::Error::PeerListIsFull);
            return_case(kf::EspNow::Error::PeerAlreadyExists);
            default:
            return_case(kf::EspNow::Error::UnknownError);
        }
    }

#undef return_case
};

}// namespace kf
