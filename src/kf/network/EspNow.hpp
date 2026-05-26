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
#include "kf/Option.hpp"
#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/io/Writable.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/ArrayString.hpp"
#include "kf/memory/Map.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Quitable.hpp"
#include "kf/mixin/Singleton.hpp"
#include "kf/primitives.hpp"

namespace kf::network {

struct EspNow;

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

}// namespace internal

/// @brief Encapsulates ESP-NOW protocol in safe C++ abstractions
/// @note Singleton wrapper for ESP-NOW API with peer management and callbacks
struct EspNow final :

    kf::mixin::Singleton<EspNow>,
    kf::mixin::Initable<EspNow, Result<void, internal::EspNowError>>,
    kf::mixin::Quitable<EspNow>

{

    /// @brief MAC address type (6 bytes)
    using Mac = memory::Array<u8, ESP_NOW_ETH_ALEN>;

    /// @brief Handler type for receiving data from unknown peers
    using ReceiveFromUnknownCallback = Function<void(const Mac &, Slice<const u8>)>;

    /// @brief ESP-NOW operation error codes
    using Error = internal::EspNowError;

    /// @brief ESP-NOW peer representation with communication capabilities
    struct Peer final : io::Writable<Peer, kf::Result<void, Error>>, mixin::NonCopyable {
        /// @brief Handler type for receiving data from this specific peer
        using ReceiveCallback = Function<void(Slice<const u8>)>;

        /// @brief Peer context storing handler and state
        struct Context {
            ReceiveCallback on_receive{nullptr};///< Callback for received data
        };

        /// @brief Add new peer to ESP-NOW network
        /// @param mac MAC address of peer to add
        /// @return Peer object on success, Error on failure
        /// @note Automatically registers peer with ESP-NOW subsystem
        [[nodiscard]] static Result<Peer, Error> add(const Mac &mac) noexcept {
            esp_now_peer_info_t peer = {
                .channel = 0,
                .ifidx = WIFI_IF_STA,
                .encrypt = false,
            };
            std::copy(mac.begin(), mac.end(), peer.peer_addr);

            const auto result = esp_now_add_peer(&peer);

            if (ESP_OK == result) {
                return ok(std::move(Peer{mac}));
            } else {
                return error(translateEspnowError(result));
            }
        }

        /// @brief Get peer MAC address
        /// @return Const reference to MAC address
        [[nodiscard]] const Mac &mac() const noexcept { return _mac; }

        /// @brief Set receive handler for this peer
        /// @param handler Callback function for incoming data
        /// @return Success or Error (PeerNotFound if peer doesn't exist)
        [[nodiscard]] Result<void, Error> onReceive(ReceiveCallback &&callback) noexcept {
            if (not exist()) { return error(Error::PeerNotFound); }

            auto &espnow = EspNow::instance();
            auto context = espnow.getPeerContext(_mac);

            if (context.isNone()) {
                espnow._peer_contexts.insert({_mac, Context{std::move(callback)}});
            } else {
                context.value().on_receive = std::move(callback);
            }

            return ok();
        }

        /// @brief Remove peer from ESP-NOW network
        /// @return Success or Error
        /// @note Also removes any associated receive handler
        [[nodiscard]] Result<void, Error> del() noexcept {
            auto &espnow = EspNow::instance();

            if (espnow.getPeerContext(_mac).isSome()) {
                espnow._peer_contexts.erase(_mac);
            }

            const auto result = esp_now_del_peer(_mac.data());

            if (ESP_OK == result) {
                return ok();
            } else {
                return error(translateEspnowError(result));
            }
        }

        /// @brief Check if peer exists in ESP-NOW network
        /// @return true if peer is registered with ESP-NOW
        [[nodiscard]] bool exist() noexcept {
            return esp_now_is_peer_exist(_mac.data());
        }

    private:
        Mac _mac;///< Peer MAC address

        /// @brief Private constructor (use Peer::add)
        explicit Peer(const Mac &mac) noexcept : _mac{mac} {}

        // impl
        using This = Peer;

        KF_IMPL_WRITABLE(This, Result<void, Error>);

        /// @brief Internal send implementation
        /// @return Success or translated ESP-NOW error
        [[nodiscard]] Result<void, Error> processSend(const void *data, usize len) noexcept {
            const auto result = esp_now_send(_mac.data(), static_cast<const u8 *>(data), len);

            if (ESP_OK == result) {
                return ok();
            } else {
                return error(translateEspnowError(result));
            }
        }

        [[nodiscard]] Result<void, Error> writeBufferImpl(Slice<const u8> buffer) {
            if (buffer.size() > ESP_NOW_MAX_DATA_LEN) { return error(Error::TooBigMessage); }
            return processSend(buffer.data(), buffer.size());
        }

        template<typename T> [[nodiscard]] Result<void, Error> writePacketImpl(T &&packet) {
            static_assert(sizeof(T) < ESP_NOW_MAX_DATA_LEN, "Message is too big!");
            return processSend(static_cast<const void *>(&packet), sizeof(T));
        }

        template<typename T> [[nodiscard]] Result<void, Error> writeMixedImpl(T &&header, Slice<const u8> buffer) {
            const auto mixed_size = sizeof(T) + buffer.size();
            u8 mixed[mixed_size];

            const auto header_data = reinterpret_cast<const u8 *>(&header);
            std::copy(header_data, header_data + sizeof(T), mixed);
            std::copy(buffer.begin(), buffer.end(), mixed + sizeof(T));

            return processSend(static_cast<const void *>(mixed), mixed_size);
        }
    };

    /// @brief Get local device MAC address
    /// @return Const reference to MAC address
    [[nodiscard]] const Mac &mac() const noexcept { return _local_mac; }

    /// @brief Set handler for receiving data from unknown peers
    /// @param handler Callback function for unknown peer data
    void onReceiveFromUnknown(ReceiveFromUnknownCallback &&callback) noexcept {
        _on_receive_from_unknown = std::move(callback);
    }

private:
    memory::Map<Mac, Peer::Context> _peer_contexts{};            ///< Map of known peers and their contexts
    ReceiveFromUnknownCallback _on_receive_from_unknown{nullptr};///< Handler for unknown peers

    /// @brief Local device MAC address (cached)
    const Mac _local_mac{
        []() -> Mac {
            Mac ret{};
            esp_read_mac(ret.data(), ESP_MAC_WIFI_STA);
            return ret;
        }()};

    /// @brief ESP-NOW receive callback (static wrapper)
    /// @param raw_mac_address Source MAC address
    /// @param data Received data buffer
    /// @param size Size of received data
    static void onReceive(const u8 *raw_mac_address, const u8 *data, int size) noexcept {
        auto &self = EspNow::instance();

        Mac source_mac;
        std::copy(raw_mac_address, raw_mac_address + ESP_NOW_ETH_ALEN, source_mac.begin());

        const Slice<const u8> buffer{data, static_cast<usize>(size)};

        const auto peer_context = self.getPeerContext(source_mac);

        if (peer_context.isNone()) {
            if (self._on_receive_from_unknown) {
                self._on_receive_from_unknown(source_mac, buffer);
            }
        } else {
            if (peer_context.value().on_receive) {
                peer_context.value().on_receive(buffer);
            }
        }
    }

    /// @brief Get peer context by MAC address
    /// @param peer_mac MAC address to look up
    [[nodiscard]] Option<Peer::Context &> getPeerContext(const Mac &peer_mac) noexcept {
        auto it = _peer_contexts.find(peer_mac);
        if (it == _peer_contexts.end()) {
            return none;
        } else {
            return someRef(it->second);
        }
    };

    // impl

    using This = EspNow;

    using InitResult = Result<void, Error>;
    KF_IMPL_INITABLE(This, InitResult);
    /// @note Sets WiFi to station mode and registers receive callback
    InitResult initImpl() noexcept {
        const auto wifi_ok = WiFiClass::mode(WIFI_MODE_STA);
        if (not wifi_ok) { return error(Error::InternalError); }

        const auto init_result = esp_now_init();
        if (ESP_OK != init_result) { return error(translateEspnowError(init_result)); }

        const auto handler_result = esp_now_register_recv_cb(onReceive);
        if (ESP_OK != handler_result) { return error(translateEspnowError(handler_result)); }

        return ok();
    }

    KF_IMPL_QUITABLE(This);
    void quitImpl() noexcept {
        // Unregisters callbacks and deinitializes ESP-NOW
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
    /// @return ArrayString with formatted MAC address (XX:XX:XX:XX:XX:XX format)
    [[nodiscard]] static memory::ArrayString<mac_string_size> stringFromMac(const Mac &mac) noexcept {
        return memory::ArrayString<mac_string_size>::formatted("%02x%02x-%02x%02x-%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
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