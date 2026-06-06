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
    auto peer_add_result = EspNow::Peer::create(mac_address);
    if (peer_add_result.isOk()) {
        return kf::some(std::move(peer_add_result.ok()));
    } else {
        // Logging at application level, not inside library.
        Serial.printf("Failed to create '%s' peer: %s\n", mac_address.toString().data(), peer_add_result.error().toString().data());
        return kf::none;
    }
}

// Callback for unknown peers - registered via onReceiveFromUnknown.
void onUnknown(const MacAddress &mac, kf::Slice<const kf::u8> data) {
    Serial.printf("(unknown): from [%s] got %d bytes\n", mac.toString().data(), data.size());
}

// Callback for the target peer - set via Peer::callback.
void onTarget(kf::Slice<const kf::u8> data) {
    Serial.printf("target: got %d bytes\n", data.size());
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

    // Direct overload used - no Option wrapper needed here.
    EspNow::instance().onReceiveFromUnknown(onUnknown);
    broadcast_peer = createPeer({0xff, 0xff, 0xff, 0xff, 0xff, 0xff});

    target_peer = createPeer({0x01, 0x02, 0x03, 0x04, 0x05, 0x06});// replace with real MAC
    if (target_peer.isSome()) {
        // Callback accepts Option; passing none would clear it.
        target_peer.unwrap().callback(onTarget);
    }
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