// KiraFlux-Toolkit Example 'network/espnow-ping'

#include <kf/Option.hpp>
#include <kf/Slice.hpp>
#include <kf/main.hpp>
#include <kf/primitives.hpp>
#include <kf/rtos/Clock.hpp>
#include <kf/BytesView.hpp>
#include <kf/rtos/Task.hpp>

#include <WiFi.h>// ESP-NOW requires STA mode – set explicitly
#include <kf/MacAddress.hpp>
#include <kf/esp/Espnow.hpp>

using kf::MacAddress;
using kf::esp::Espnow;

// --- Globals ---

kf::Option<Espnow::Peer> broadcast_peer{kf::none};
kf::Option<Espnow::Peer> target_peer{kf::none};

// --- Factory function ---

kf::Option<Espnow::Peer> createPeer(kf::Init &init, MacAddress const &mac_address) noexcept {
    auto result = Espnow::Peer::create(Espnow::Peer::Config{
        .mac_address = mac_address,
        .wifi_interface_sta = true,
    });

    if (result.isOk()) {
        return kf::some(std::move(result.ok()));
    }

    init.logger.warn("Failed to create peer {}: {}", mac_address, result.error());
    return kf::none;
}

// --- Callback ---

void onReceive(kf::Init &init, MacAddress const &mac, kf::Slice<kf::u8 const> data) {
    if (target_peer.isSome() and target_peer.unwrap().mac() == mac) {
        init.logger.info("target: got {} bytes", data.length());
    }

    init.logger.debug("(unknown): from {} got {} bytes", mac, data.length());
}

// --- Main application ---

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: network/espnow-ping");

    // --- WiFi setup ---

    if (not WiFiClass::mode(WIFI_MODE_STA)) {
        init.logger.error("Failed to set Wi-Fi mode to STA");
        return;
    }

    // --- ESP-NOW initialization ---

    auto init_result = Espnow::instance().init();
    if (init_result.isError()) {
        init.logger.error("ESP-NOW init failed: {}", init_result.error());
        return;
    }
    init.logger.info("ESP-NOW initialized successfully");

    // --- Self MAC address ---

    MacAddress const &self_mac = Espnow::instance().mac();
    init.logger.info("Self MAC: {}", self_mac);

    // --- Callback registration ---

    Espnow::instance().callback([&](MacAddress const &mac, BytesView data) {
        onReceive(init, mac, data);
    });

    // --- Create peers ---

    broadcast_peer = createPeer(init, {0xff, 0xff, 0xff, 0xff, 0xff, 0xff});
    target_peer = createPeer(init, {0x01, 0x02, 0x03, 0x04, 0x05, 0x06});// replace with real MAC

    // --- Main loop ---

    while (true) {
        if (broadcast_peer.isSome()) {
            init.logger.info("Sending: broadcast");
            auto const result = broadcast_peer.unwrap().writePacket("[broadcast]: ping");
            if (result.isError()) {
                init.logger.error("[broadcast]: send failed: {}", result.error());
            }
        }

        if (target_peer.isSome()) {
            init.logger.info("Sending: target");
            auto const result = target_peer.unwrap().writePacket("ping");
            if (result.isError()) {
                init.logger.error("[target]: send failed: {}", result.error());
            }
        }

        rtos::Task::sleep(5000);
        init.logger.debug("Tick: {}", rtos::Clock::now());
    }
}