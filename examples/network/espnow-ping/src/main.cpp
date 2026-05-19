// KiraFlux-Toolkit Demo 'espnow'
#include <Arduino.h>
#include <kf/primitives.hpp>
#include <kf/memory/Slice.hpp>

#include <kf/network/EspNow.hpp>

using kf::network::EspNow;

constexpr EspNow::Mac broadcast_address{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
constexpr EspNow::Mac target_peer_address{0x01, 0x02, 0x03, 0x04, 0x05, 0x06};// replace with real peer MAC address

kf::Option<EspNow::Peer> broadcast_peer{}, target_peer{};

void onUnknown(const EspNow::Mac &mac, kf::memory::Slice<const kf::u8> data) {
    Serial.printf("(unknown): from [%s] got %d bytes\n", EspNow::stringFromMac(mac).data(), data.size());
}

void setupBroadcastPeer() {
    EspNow::instance().onReceiveFromUnknown(onUnknown);

    auto result = EspNow::Peer::add(broadcast_address);
    if (result.isError()) {
        Serial.printf("Failed to add broadcast peer: %s\n", EspNow::stringFromError(result.error()));
    }

    if (result.isOk()) {
        broadcast_peer.value(std::move(result.value()));
    }
}

void onTarget(kf::memory::Slice<const kf::u8> data) {
    Serial.printf("from [%s] got %d bytes\n", EspNow::stringFromMac(target_peer_address).data(), data.size());
}

void setupTargetPeer() {
    EspNow::instance().onReceiveFromUnknown(onUnknown);

    auto peer_add_result = EspNow::Peer::add(target_peer_address);
    if (peer_add_result.isError()) {
        Serial.printf("Failed to add broadcast peer: %s\n", EspNow::stringFromError(peer_add_result.error()));
    } else {
        // ok => attach handler

        const auto result = peer_add_result.value().onReceive(onTarget);
        if (result.isError()) {
            Serial.printf("Failed to attach receive callback: %s\n", EspNow::stringFromError(result.error()));
        }
    }

    if (peer_add_result.isOk()) {
        target_peer.value(std::move(peer_add_result.value()));
    }
}

void setup() {
    Serial.begin(115200);
    Serial.printf("Self: %s\n", EspNow::stringFromMac(EspNow::instance().mac()).data());

    const auto init_error_option = EspNow::instance().init();
    if (init_error_option.isError()) {
        Serial.println(EspNow::stringFromError(init_error_option.error()));
        return;
    }

    setupBroadcastPeer();
    setupTargetPeer();
}

void loop() {
    if (broadcast_peer.isSome()) {
        Serial.println("Sending: broadcast");
        const auto result = broadcast_peer.value().writePacket("[broadcast]: ping");
        if (result.isError()) { Serial.println("[broadcast]: Failed to send"); }
    }

    if (target_peer.isSome()) {
        Serial.println("Sending: target");
        const auto result = target_peer.value().writePacket("ping");
        if (result.isError()) { Serial.println("Failed to send"); }
    }

    delay(5000);
    Serial.println(millis());
}
