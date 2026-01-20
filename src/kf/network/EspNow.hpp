// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <esp_mac.h>
#include <esp_now.h>

#include <WiFi.h>

#include "kf/Function.hpp"
#include "kf/Result.hpp"
#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/core/utility.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/Map.hpp"
#include "kf/memory/Slice.hpp"
#include "kf/memory/ArrayString.hpp"
#include "kf/pattern/Singleton.hpp"


namespace kf {

/// @brief Encapsulates ESP-NOW protocol in safe C++ abstractions
/// @note Singleton wrapper for ESP-NOW API with peer management and callbacks
struct EspNow : Singleton<EspNow> {
    friend struct Singleton<EspNow>;

    using Mac = Array<u8, ESP_NOW_ETH_ALEN>;///< MAC address type (6 bytes)

    /// @brief Handler type for receiving data from unknown peers
    using UnknownReceiveHandler = Function<void(const Mac &, const Slice<const u8>)>;

    /// @brief ESP-NOW operation error codes
    enum class Error : u8 {
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

    /// @brief ESP-NOW peer representation with communication capabilities
    struct Peer {
        /// @brief Handler type for receiving data from this specific peer
        using ReceiveHandler = Function<void(Slice<const u8>)>;

        /// @brief Peer context storing handler and state
        struct Context {
            ReceiveHandler on_receive{nullptr};///< Callback for received data
        };

    private:
        Mac mac_;///< Peer MAC address

    public:
        /// @brief Add new peer to ESP-NOW network
        /// @param mac MAC address of peer to add
        /// @return Peer object on success, Error on failure
        /// @note Automatically registers peer with ESP-NOW subsystem
        kf_nodiscard static Result<Peer, Error> add(const Mac &mac) {
            esp_now_peer_info_t peer = {
                .channel = 0,
                .ifidx = WIFI_IF_STA,
                .encrypt = false,
            };
            std::copy(mac.begin(), mac.end(), peer.peer_addr);

            const auto result = esp_now_add_peer(&peer);

            if (ESP_OK == result) {
                return {Peer{mac}};
            } else {
                return {translateEspnowError(result)};
            }
        }

        /// @brief Get peer MAC address
        /// @return Const reference to MAC address
        kf_nodiscard const Mac &mac() const { return mac_; }

        /// @brief Send typed packet to peer
        /// @tparam T Type of data to send (must fit in ESP_NOW_MAX_DATA_LEN)
        /// @param value Data to send
        /// @return Success or Error
        /// @note Automatically checks size constraint at compile time
        template<typename T> kf_nodiscard Result<void, Error> sendPacket(const T &value) {
            static_assert(sizeof(T) < ESP_NOW_MAX_DATA_LEN, "Message is too big!");
            return processSend(static_cast<const void *>(&value), sizeof(T));
        }

        /// @brief Send raw buffer to peer
        /// @param buffer Data slice to send
        /// @return Success or Error
        /// @note Checks size constraint at runtime
        kf_nodiscard Result<void, Error> sendBuffer(Slice<const u8> buffer) {
            if (buffer.size() > ESP_NOW_MAX_DATA_LEN) {
                return {Error::TooBigMessage};
            }

            return processSend(buffer.data(), buffer.size());
        }

        /// @brief Set receive handler for this peer
        /// @param handler Callback function for incoming data
        /// @return Success or Error (PeerNotFound if peer doesn't exist)
        kf_nodiscard Result<void, Error> setReceiveHandler(ReceiveHandler &&handler) {
            if (not exist()) {
                return {Error::PeerNotFound};
            }

            auto &espnow = EspNow::instance();
            auto context = espnow.getPeerContext(mac_);

            if (nullptr == context) {
                espnow.peer_contexts.insert({mac_, Context{kf::move(handler)}});
            } else {
                context->on_receive = kf::move(handler);
            }

            return {};
        }

        /// @brief Remove peer from ESP-NOW network
        /// @return Success or Error
        /// @note Also removes any associated receive handler
        kf_nodiscard Result<void, Error> del() {
            auto &espnow = EspNow::instance();

            if (nullptr != espnow.getPeerContext(mac_)) {
                espnow.peer_contexts.erase(mac_);
            }

            const auto result = esp_now_del_peer(mac_.data());

            if (ESP_OK == result) {
                return {};
            } else {
                return {translateEspnowError(result)};
            }
        }

        /// @brief Check if peer exists in ESP-NOW network
        /// @return true if peer is registered with ESP-NOW
        kf_nodiscard bool exist() {
            return esp_now_is_peer_exist(mac_.data());
        }

    private:
        /// @brief Internal send implementation
        /// @param data Pointer to data buffer
        /// @param len Size of data in bytes
        /// @return Success or translated ESP-NOW error
        kf_nodiscard Result<void, Error> processSend(const void *data, usize len) {
            const auto result = esp_now_send(
                mac_.data(),
                static_cast<const u8 *>(data),
                len);

            if (ESP_OK == result) {
                return {};
            } else {
                return {translateEspnowError(result)};
            }
        }

        /// @brief Private constructor (use Peer::add)
        explicit Peer(const Mac &mac) :
            mac_{mac} {}
    };

private:
    Map<Mac, Peer::Context> peer_contexts{};               ///< Map of known peers and their contexts
    UnknownReceiveHandler unknown_receive_handler{nullptr};///< Handler for unknown peers

    /// @brief Local device MAC address (cached)
    const Mac mac_{
        []() -> Mac {
            Mac ret{};
            esp_read_mac(ret.data(), ESP_MAC_WIFI_STA);
            return ret;
        }()
    };

public:
    /// @brief Initialize ESP-NOW protocol
    /// @return Success or Error
    /// @note Sets WiFi to station mode and registers receive callback
    kf_nodiscard static Result<void, Error> init() {
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

    /// @brief Deinitialize ESP-NOW protocol
    /// @note Unregisters callbacks and deinitializes ESP-NOW
    static void quit() {
        (void) esp_now_unregister_recv_cb();
        (void) esp_now_deinit();
    }

    /// @brief Get local device MAC address
    /// @return Const reference to MAC address
    kf_nodiscard const Mac &mac() const { return mac_; }

    /// @brief Set handler for receiving data from unknown peers
    /// @param handler Callback function for unknown peer data
    void setUnknownReceiveHandler(UnknownReceiveHandler &&handler) {
        unknown_receive_handler = kf::move(handler);
    }

private:
    /// @brief ESP-NOW receive callback (static wrapper)
    /// @param raw_mac_address Source MAC address
    /// @param data Received data buffer
    /// @param size Size of received data
    static void onReceive(const u8 *raw_mac_address, const u8 *data, int size) {
        auto &self = EspNow::instance();

        const auto &source_address = *reinterpret_cast<const Mac *>(raw_mac_address);
        const Slice<const u8> buffer{data, static_cast<usize>(size)};

        const auto peer_context = self.getPeerContext(source_address);

        if (nullptr == peer_context) {
            if (nullptr == self.unknown_receive_handler) { return; }
            self.unknown_receive_handler(source_address, buffer);
        } else {
            if (nullptr == peer_context->on_receive) { return; }
            peer_context->on_receive(buffer);
        }
    }

    /// @brief Get peer context by MAC address
    /// @param peer_mac MAC address to look up
    /// @return Pointer to peer context or nullptr if not found
    kf_nodiscard Peer::Context *getPeerContext(const Mac &peer_mac) {
        auto it = peer_contexts.find(peer_mac);
        if (it == peer_contexts.end()) {
            return nullptr;
        } else {
            return &it->second;
        }
    };

    /// @brief Translate ESP error code to Error enum
    /// @param result ESP error code
    /// @return Corresponding Error enum value
    kf_nodiscard static Error translateEspnowError(esp_err_t result) {
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
    kf_nodiscard static ArrayString<mac_string_size> stringFromMac(const Mac &mac) {
        ArrayString<mac_string_size> ret{};
        const auto p = mac.data();
        (void) ret.format("%02x%02x-%02x%02x-%02x%02x", p[0], p[1], p[2], p[3], p[4], p[5]);
        return ret;
    }

#define return_case(__v) \
    case __v: return #__v

    /// @brief Convert Error enum to string representation
    /// @param error Error code to convert
    /// @return String representation of error
    kf_nodiscard static const char *stringFromError(kf::EspNow::Error error) {
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