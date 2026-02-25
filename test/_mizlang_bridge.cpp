#include <Arduino.h>

#include <kf/network/MizLangBridge.hpp>

using Bridge = kf::MizLangBridge<1, kf::u8>;
Bridge bridge{
    kf::io::InputStream(Serial),
    kf::io::OutputStream(Serial),
    {
        [](kf::io::InputStream &) -> kf::Result<void, Bridge::Error> {
            return {};
        },
    }
    //
};

auto ins_0 = bridge.createInstruction([](kf::io::OutputStream &, void *) -> kf::Result<void, Bridge::Error> {
    return {};
});

auto ins_1 = bridge.createInstruction([](kf::io::OutputStream &, void *args) -> kf::Result<void, Bridge::Error> {
    return {};
});

void setup() {
    (void)ins_0.send(nullptr);
    (void)ins_1.send(nullptr);
}

void loop() {
}