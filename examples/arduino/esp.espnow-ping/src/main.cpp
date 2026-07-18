// KiraFlux-Toolkit Demo 'espnow-ping'

#include <kf/main.hpp>

#include <WiFi.h>// ESP‑NOW requires STA mode. Set explicitly - library does not manage WiFi.
#include <kf/MacAddress.hpp>
#include <kf/esp/Espnow.hpp>

#include <kf/Option.hpp>
#include <kf/Slice.hpp>
#include <kf/primitives.hpp>

using kf::MacAddress;
using kf::esp::Espnow;

// Option because Peer may be absent (not created or creation failed). Peer is move‑only.
kf::Option<Espnow::Peer> broadcast_peer{kf::none}, target_peer{kf::none};

// Factory returning Option<Peer>. Not Result - absence is a valid state (not an error).
kf::Option<Espnow::Peer> createPeer(const MacAddress &mac_address) noexcept {
    auto peer_add_result = Espnow::Peer::create(Espnow::Peer::Config{
        .mac_address = mac_address,
        .wifi_interface_sta = true,
    });
    if (peer_add_result.isOk()) {
        return kf::some(std::move(peer_add_result.ok()));
    } else {
        // Logging at application level, not inside library.
        Serial.printf("Failed to create '%s' peer: %s\n", mac_address.repr().data(), peer_add_result.error().repr().data());
        return kf::none;
    }
}

// Callback for unknown peers - registered via Espnow::callback.
void onReceive(const MacAddress &mac, kf::Slice<const kf::u8> data) {
    if (target_peer.isSome() and target_peer.unwrap().mac() == mac) {
        Serial.printf("target: got %d bytes\n", data.length());
    }

    Serial.printf("(unknown): from [%s] got %d bytes\n", mac.repr().data(), data.length());
}

void kf::main(kf::Init &init) {

    // WiFi is configured explicitly before Espnow::init() because the application may have its own WiFi setup.
    if (not WiFiClass::mode(WIFI_MODE_STA)) {
        Serial.println("Wifi mode set failed");
        return;
    }

    const auto init_error_option = Espnow::instance().init();
    if (init_error_option.isError()) {
        Serial.println(init_error_option.error().repr().data());
        return;
    }

    const MacAddress &self_mac_address = Espnow::instance().mac();
    Serial.printf("Self: %s\n", self_mac_address.repr().data());

    // Callback accepts Function, Option or none; passing none would clear it.
    Espnow::instance().callback(onReceive);
    broadcast_peer = createPeer({0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
    target_peer = createPeer({0x01, 0x02, 0x03, 0x04, 0x05, 0x06});// replace with real MAC

    while (true) {

        // isSome() must be checked before unwrap(), otherwise abort().

        if (broadcast_peer.isSome()) {
            Serial.println("Sending: broadcast");

            const auto result = broadcast_peer.unwrap().writePacket("[broadcast]: ping");
            if (result.isError()) {
                // Error provides repr() - convenient for logging.
                Serial.printf("[broadcast]: Failed to send: %s\n", result.error().repr().data());
            }
        }

        if (target_peer.isSome()) {
            Serial.println("Sending: target");

            const auto result = target_peer.unwrap().writePacket("ping");
            if (result.isError()) {
                Serial.printf("Failed to send: %s\n", result.error().repr().data());
            }
        }

        delay(5000);
        Serial.println(millis());
    }
}
