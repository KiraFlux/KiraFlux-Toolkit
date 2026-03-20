#include <Arduino.h>
#include <kf/network/MizLangBridge.hpp>

using kf::memory::io::InputStream;
using kf::memory::io::OutputStream;

using Bridge = kf::network::MizLangBridge<1, kf::u8, kf::u8>;// < handlers_total, local_code, [remote_code = local_code] >
Bridge bridge{
    InputStream{Serial},
    OutputStream{Serial},

    // instruction table with (1) handler
    {
        // #0
        [](InputStream &) -> kf::Result<void, Bridge::Error> {
            return {};
        },
    },
};

auto ins_0 = bridge.createInstruction([](OutputStream &, void *) -> kf::Result<void, Bridge::Error> {
    // do something ...

    return {};// OK
});

struct MyArgs {
    int a, b;
};

auto ins_1 = bridge.createInstruction([](OutputStream &, void *args) -> kf::Result<void, Bridge::Error> {
    if (nullptr == args) {
        return Bridge::Error::Sender_ArgumentWriteFail;
    }

    // do something ...

    return {};
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

void loop() {}