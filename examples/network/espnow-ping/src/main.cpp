// KiraFlux-Toolkit Demo 'espnow-ping'

#include <Arduino.h>
#include <WiFi.h>// ESP‑NOW requires STA mode. Set explicitly - library does not manage WiFi.
#include <kf/Option.hpp>
#include <kf/Slice.hpp>
#include <kf/primitives.hpp>

#include <kf/network/EspNow.hpp>
#include <kf/network/MacAddress.hpp>

using kf::network::EspNow;
using kf::network::MacAddress;

// Option because Peer may be absent (not created or creation failed). Peer is move‑only.
kf::Option<EspNow::Peer> broadcast_peer{kf::none}, target_peer{kf::none};

// Factory returning Option<Peer>. Not Result - absence is a valid state (not an error).
kf::Option<EspNow::Peer> createPeer(const MacAddress &mac_address) noexcept {
    auto peer_add_result = EspNow::Peer::create(EspNow::Peer::Config{
        .mac_address = mac_address,
        .wifi_interface_sta = true,
    });
    if (peer_add_result.isOk()) {
        return kf::some(std::move(peer_add_result.ok()));
    } else {
        // Logging at application level, not inside library.
        Serial.printf("Failed to create '%s' peer: %s\n", mac_address.toString().data(), peer_add_result.error().toString().data());
        return kf::none;
    }
}

// Callback for unknown peers - registered via EspNow::callback.
void onReceive(const MacAddress &mac, kf::Slice<const kf::u8> data) {
    if (target_peer.isSome() and target_peer.unwrap().mac() == mac) {
        Serial.printf("target: got %d bytes\n", data.length());
    }

    Serial.printf("(unknown): from [%s] got %d bytes\n", mac.toString().data(), data.length());
}

void setup() {
    Serial.begin(115200);

    // WiFi is configured explicitly before EspNow::init() because the application may have its own WiFi setup.
    if (not WiFiClass::mode(WIFI_MODE_STA)) {
        Serial.println("Wifi mode set failed");
        return;
    }

    const auto init_error_option = EspNow::instance().init();
    if (init_error_option.isError()) {
        Serial.println(init_error_option.error().toString().data());
        return;
    }

    const MacAddress &self_mac_address = EspNow::instance().mac();
    Serial.printf("Self: %s\n", self_mac_address.toString().data());

    // Callback accepts Function, Option or none; passing none would clear it.
    EspNow::instance().callback(onReceive);
    broadcast_peer = createPeer({0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
    target_peer = createPeer({0x01, 0x02, 0x03, 0x04, 0x05, 0x06});// replace with real MAC
}

void loop() {
    // isSome() must be checked before unwrap(), otherwise abort().
    if (broadcast_peer.isSome()) {
        Serial.println("Sending: broadcast");
        const auto result = broadcast_peer.unwrap().writePacket("[broadcast]: ping");
        if (result.isError()) {
            // Error provides toString() - convenient for logging.
            Serial.printf("[broadcast]: Failed to send: %s\n", result.error().toString().data());
        }
    }

    if (target_peer.isSome()) {
        Serial.println("Sending: target");
        const auto result = target_peer.unwrap().writePacket("ping");
        if (result.isError()) {
            Serial.printf("Failed to send: %s\n", result.error().toString().data());
        }
    }

    delay(5000);
    Serial.println(millis());
}