// KiraFlux-Toolkit Demo 'espnow'
#include <Arduino.h>
#include <kf/aliases.hpp>
#include <kf/memory/Slice.hpp>

#include <kf/network/EspNow.hpp>

using kf::network::EspNow;

constexpr EspNow::Mac broadcast_address{0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr EspNow::Mac other_peer_address{0x01, 0x02, 0x03, 0x04, 0x05, 0x06};// replace with real peer MAC address

kf::Option<EspNow::Peer> broadcast_peer{}, target_peer{};

void setup() {
    Serial.begin(115200);

    // 1. Init espnow
    const auto init_error_option = EspNow::init().error();
    if (init_error_option.hasValue()) {
        Serial.println(EspNow::stringFromError(init_error_option.value()));
        return;
    }

    // 2. Set fallback handler
    EspNow::instance().onReceiveFromUnknown([](const EspNow::Mac &mac, const kf::memory::Slice<const kf::u8> data) {
        Serial.printf("from [%s] got %d bytes", EspNow::stringFromMac(mac).data(), data.size());
        return;
    });

    // 3. Add broadcast peer
    const auto broadcast_peer_add_result = EspNow::Peer::add(broadcast_address);
    if (broadcast_peer_add_result.isError()) {
        Serial.print("Failed to add broadcast peer: ");
        Serial.println(EspNow::stringFromError(broadcast_peer_add_result.error().value()));
        return;
    }

    broadcast_peer = broadcast_peer_add_result.ok();

    // 4. Add target peer
    const auto other_peer_add_result = EspNow::Peer::add(broadcast_address);
    if (other_peer_add_result.isError()) {
        Serial.print("Failed to add other peer: ");
        Serial.println(EspNow::stringFromError(other_peer_add_result.error().value()));
        return;
    } else {
        other_peer_add_result.ok().value().onReceive([](const kf::memory::Slice<const kf::u8> data) {
            Serial.printf("from [%s] got %d bytes", EspNow::stringFromMac(other_peer_address).data(), data.size());
        });
    }

    target_peer = other_peer_add_result.ok();
}

void loop() {
    delay(5000);

    if (broadcast_peer.hasValue()) {
        broadcast_peer.value().sendPacket("[broadcast]: ping");
    }

    if (target_peer.hasValue()) {
        target_peer.value().sendPacket("ping");
    }
}
