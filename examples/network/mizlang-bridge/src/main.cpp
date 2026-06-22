#include <Arduino.h>

#include <kf/io/ArduinoStream.hpp>
#include <kf/memory/Array.hpp>
#include <kf/network/MizLangBridge.hpp>

using kf::io::ArduinoStream;

using Bridge = kf::network::MizLangBridge<ArduinoStream, ArduinoStream, kf::u8, kf::u8>;// < Input, Output, local_code, [remote_code = local_code] >

// may be shared with some amount of bridge instances
kf::memory::Array<Bridge::ReceiveFunctionType, 1> receiver_table{
    // instruction table with (1) handler
    {
        // #0
        [](ArduinoStream &) -> kf::Result<void, Bridge::Error> {
            return kf::ok();
        },
    },
};

Bridge my_bridge{
    ArduinoStream{Serial},
    ArduinoStream{Serial},
    receiver_table.slice(),
};

auto ins_0 = my_bridge.createInstruction([](ArduinoStream &, void *) -> kf::Result<void, Bridge::Error> {
    // do something ...

    return kf::ok();
});

struct MyArgs {
    int a, b;
};

auto ins_1 = my_bridge.createInstruction([](ArduinoStream &, void *args) -> kf::Result<void, Bridge::Error> {
    if (nullptr == args) {
        return kf::error(Bridge::Error::Sender_ArgumentWriteFail);
    }

    // do something ...

    return kf::ok();
});

void setup() {

    // 1. Send instruction #0

    (void) ins_0.send(nullptr);

    // 2. Send instruction #1

    MyArgs args{
        .a = 123,
        .b = 456,
    };

    const auto send_result = ins_1.send(&args);

    if (send_result.isError()) {
        // send failed.

        const Bridge::Error error_kind = send_result.error();// if result.isError() => error() is some and has value() garanteed.
    }
}

void loop() {
    const auto result = my_bridge.poll();// polling: handle remote instructions
}