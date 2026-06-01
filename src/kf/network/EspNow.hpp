// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

// std
#include <utility>

// esp
#include <esp_mac.h>
#include <esp_now.h>

// arduino
#include <WiFi.h>

// lib
#include "kf/Function.hpp"
#include "kf/NoneType.hpp"
#include "kf/Option.hpp"
#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/io/Writable.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/StaticString.hpp"
#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"
#include "kf/mixin/Singleton.hpp"
#include "kf/network/MacAddress.hpp"
#include "kf/primitives.hpp"

namespace kf::network {

namespace internal {

enum class EspNowError : u8 {
    InternalError,    ///< ESP-NOW internal API error
    UnknownError,     ///< Unknown ESP API error
    NotInitialized,   ///< ESP-NOW protocol not initialized
    IncorrectWiFiMode,///< Incorrect WiFi interface mode set
    PeerListIsFull,   ///< Peer list is at maximum capacity
    InvalidArg,       ///< Invalid argument passed to API
    NoMemory,         ///< Insufficient memory for peer addition
    PeerAlreadyExists,///< Peer already exists in list
    PeerNotFound,     ///< Peer not found in peer list
    TooBigMessage,    ///< Message size exceeds ESP_NOW_MAX_DATA_LEN
};

struct MacAddressed {
    explicit constexpr MacAddressed(const MacAddress &mac_address) noexcept : _mac_address{mac_address} {}

    /// @brief Get MAC address
    [[nodiscard]] const MacAddress &mac() const noexcept { return _mac_address; }

protected:
    MacAddress _mac_address;
};

}// namespace internal

/// @brief Encapsulates ESP-NOW protocol in safe C++ abstractions
/// @note Singleton wrapper for ESP-NOW API with peer management and callbacks
struct EspNow final :

    internal::MacAddressed,
    kf::mixin::Singleton<EspNow>,
    kf::mixin::Initable<EspNow, Result<void, internal::EspNowError>>,
    kf::mixin::Quitable<EspNow>

{

    static constexpr usize max_peers{20};

    /// @brief Handler type for receiving data from unknown peers
    using ReceiveFromUnknownCallback = Function<void(const MacAddress &, Slice<const u8>)>;

    /// @brief ESP-NOW operation error codes
    using Error = internal::EspNowError;

    /// @brief ESP-NOW peer representation with communication capabilities
    struct Peer final :

        internal::MacAddressed,
        mixin::NonCopyable,
        io::Writable<Peer, kf::Result<void, Error>>,
        mixin::Callbacked<Slice<const u8>>

    {

        struct Config final {
            wifi_interface_t wifi_interface;
            u8 channel;
            bool encrypt;

            [[nodiscard]] static constexpr Config defaults() noexcept {
                return Config{
                    .wifi_interface = WIFI_IF_STA,
                    .channel = 0,
                    .encrypt = false,
                };
            }
        };

        Peer(Peer &&other) noexcept = default;

        Peer &operator=(Peer &&other) noexcept = default;

        /// @brief Add new peer to ESP-NOW network
        /// @param mac MAC address of peer to add
        /// @return Peer object on success, Error on failure
        /// @note Automatically registers peer with ESP-NOW subsystem
        [[nodiscard]] static Result<Peer, Error> create(const MacAddress &mac, Config config = Config::defaults()) noexcept {
            esp_now_peer_info_t peer_info{
                .channel = config.channel,
                .ifidx = config.wifi_interface,
                .encrypt = config.encrypt,
            };
            std::copy(mac.begin(), mac.end(), peer_info.peer_addr);

            const auto result = esp_now_add_peer(&peer_info);

            if (ESP_OK == result) {
                return ok(std::move(Peer{mac}));
            } else {
                return error(translateEspnowError(result));
            }
        }

        /// @brief Remove peer from ESP-NOW network
        /// @return Success or Error
        /// @note Also removes any associated receive handler
        [[nodiscard]] Result<void, Error> del() noexcept {
            EspNow::instance().getPeer(_mac_address).reset();

            const auto result = esp_now_del_peer(_mac_address.data());

            if (ESP_OK == result) {
                return ok();
            } else {
                return error(translateEspnowError(result));
            }
        }

        /// @brief Check if peer exists in ESP-NOW network
        /// @return true if peer is registered with ESP-NOW
        [[nodiscard]] bool exist() noexcept {
            return esp_now_is_peer_exist(_mac_address.data());
        }

        ~Peer() noexcept {
            (void) del();
        }

    private:
        /// @brief Private constructor
        /// @see Peer::add
        explicit Peer(const MacAddress &mac) noexcept : internal::MacAddressed{mac} {
            EspNow::instance().putPeer(*this);
        }

        /// @brief Internal send implementation
        /// @return Success or translated ESP-NOW error
        [[nodiscard]] Result<void, Error> processSend(const void *data, usize len) noexcept {
            const auto result = esp_now_send(_mac_address.data(), static_cast<const u8 *>(data), len);

            if (ESP_OK == result) {
                return ok();
            } else {
                return error(translateEspnowError(result));
            }
        }

        KF_IMPL_WRITABLE(Peer, Result<void, Error>);

        [[nodiscard]] Result<void, Error> writeBufferImpl(Slice<const u8> buffer) noexcept {
            if (buffer.size() > ESP_NOW_MAX_DATA_LEN) { return error(Error::TooBigMessage); }
            return processSend(buffer.data(), buffer.size());
        }

        template<typename T> [[nodiscard]] Result<void, Error> writePacketImpl(T &&packet) noexcept {
            static_assert(sizeof(T) < ESP_NOW_MAX_DATA_LEN, "Message is too big!");
            return processSend(static_cast<const void *>(&packet), sizeof(T));
        }

        template<typename T> [[nodiscard]] Result<void, Error> writeMixedImpl(T &&header, Slice<const u8> buffer) noexcept {
            const auto mixed_size = sizeof(T) + buffer.size();
            u8 mixed[mixed_size];

            const auto header_data = reinterpret_cast<const u8 *>(&header);
            std::copy(header_data, header_data + sizeof(T), mixed);
            std::copy(buffer.begin(), buffer.end(), mixed + sizeof(T));

            return processSend(static_cast<const void *>(mixed), mixed_size);
        }
    };

    /// @brief Set handler for receiving data from unknown peers
    void onReceiveFromUnknown(Option<ReceiveFromUnknownCallback> optional_callback) noexcept {
        _on_receive_from_unknown = std::move(optional_callback);
    }

    void onReceiveFromUnknown(ReceiveFromUnknownCallback callback) noexcept {
        _on_receive_from_unknown = some(std::move(callback));
    }

    void onReceiveFromUnknown(NoneType) noexcept {
        _on_receive_from_unknown.reset();
    }

private:
    memory::Array<Option<const Peer &>, max_peers> _peers{};          /// Registry of peer entries
    Option<ReceiveFromUnknownCallback> _on_receive_from_unknown{none};///< Handler for unknown peers

    friend struct ::kf::mixin::Singleton<EspNow>;

    constexpr EspNow() noexcept : MacAddressed{{}} {}

    /// @brief ESP-NOW receive callback (static wrapper)
    /// @param raw_mac_address Source MAC address
    /// @param data Received data buffer
    /// @param size Size of received data
    static void onReceive(const u8 *raw_mac_address, const u8 *data, int size) noexcept {
        auto &self = EspNow::instance();

        MacAddress source_mac_address;
        std::copy(raw_mac_address, raw_mac_address + ESP_NOW_ETH_ALEN, source_mac_address.begin());

        const Slice<const u8> buffer{data, static_cast<usize>(size)};

        auto peer = self.getPeer(source_mac_address);

        if (peer.isNone()) {
            if (self._on_receive_from_unknown.isSome()) {
                self._on_receive_from_unknown.unwrap()(source_mac_address, buffer);
            }
        } else {
            peer.unwrap().invoke(buffer);
        }
    }

    /// @brief Get peer context by MAC address
    [[nodiscard]] Option<const Peer &> getPeer(const MacAddress &target_mac_address) noexcept {
        for (auto &peer: _peers) {
            if (peer.isSome() and peer.unwrap().mac() == target_mac_address) {
                return someRef(peer.unwrap());
            }
        }

        return none;
    };

    void putPeer(const Peer &peer_to_add) noexcept {
        for (auto &peer: _peers) {
            if (peer.isNone()) {
                peer = someRef(peer_to_add);
                return;
            }
        }
    };

    KF_IMPL_INITABLE(EspNow, Result<void, Error>);
    Result<void, Error> initImpl() noexcept {
        (void) esp_read_mac(_mac_address.data(), ESP_MAC_WIFI_STA);

        const auto wifi_ok = WiFiClass::mode(WIFI_MODE_STA);
        if (not wifi_ok) { return error(Error::InternalError); }

        const auto init_result = esp_now_init();
        if (ESP_OK != init_result) { return error(translateEspnowError(init_result)); }

        const auto handler_result = esp_now_register_recv_cb(onReceive);
        if (ESP_OK != handler_result) { return error(translateEspnowError(handler_result)); }

        return ok();
    }

    KF_IMPL_QUITABLE(EspNow);
    void quitImpl() noexcept {
        (void) esp_now_unregister_recv_cb();
        (void) esp_now_deinit();
    }

    // stringFrom...

    /// @brief Translate ESP error code to Error enum
    /// @param result ESP error code
    /// @return Corresponding Error enum value
    [[nodiscard]] static Error translateEspnowError(esp_err_t result) noexcept {
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

    static constexpr auto mac_string_size = sizeof("0000-0000-0000");///< Size for MAC string representation

public:
    /// @brief Convert MAC address to human-readable string
    /// @param mac MAC address to convert
    /// @return StaticString with formatted MAC address (XX:XX:XX:XX:XX:XX format)
    [[nodiscard]] static memory::StaticString<mac_string_size> stringFromMac(const MacAddress &mac) noexcept {
        return memory::StaticString<mac_string_size>::formatted("%02x%02x-%02x%02x-%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }

#define return_case(__v) \
    case __v: return #__v

    /// @brief Convert Error enum to string representation
    /// @param error Error code to convert
    /// @return String representation of error
    [[nodiscard]] static const char *stringFromError(kf::network::EspNow::Error error) noexcept {
        switch (error) {
            return_case(kf::network::EspNow::Error::NotInitialized);
            return_case(kf::network::EspNow::Error::InternalError);
            return_case(kf::network::EspNow::Error::TooBigMessage);
            return_case(kf::network::EspNow::Error::InvalidArg);
            return_case(kf::network::EspNow::Error::NoMemory);
            return_case(kf::network::EspNow::Error::PeerNotFound);
            return_case(kf::network::EspNow::Error::IncorrectWiFiMode);
            return_case(kf::network::EspNow::Error::PeerListIsFull);
            return_case(kf::network::EspNow::Error::PeerAlreadyExists);
            default:
                return_case(kf::network::EspNow::Error::UnknownError);
        }
    }

#undef return_case
};

}// namespace kf::network