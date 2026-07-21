// KiraFlux-Toolkit Example 'arduino/stream'

#include <kf/main.hpp>

#include <Arduino.h>
#include <kf/arduino/ArduinoStream.hpp>

using kf::arduino::ArduinoStream;

struct MyPacket {
    kf::u32 crc;
    char message[32];
};

kf::u8 buffer[128];

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: arduino/stream");

    // Wrap Arduino Stream instance
    ArduinoStream my_stream{Serial};

    // writing

    // All Write operation returns kf::Result<void, ArduinoStream::Error>
    // where ArduinoStream::Error is an enum with this options
    // - ReadNotAvailable -- on read error
    // - ReadFailed       -- on read error
    // - WriteFailed      -- on write error

    // Get available bytes for write

    (void) my_stream.availableForWrite();// -> kf::usize

    // write single byte

    (void) my_stream.writeByte(0xAA);

    // write buffer

    kf::Slice<kf::u8 const> my_buffer_view{buffer};
    (void) my_stream.writeBuffer(my_buffer_view);

    // write packet (Trivially copyable struct)

    (void) my_stream.writePacket(MyPacket{
        .crc = 12345,
        .message = "Hello",
    });

    // reading

    // Get available bytes for read

    (void) my_stream.availableForRead();// -> kf::usize

    // read byte

    kf::Result<kf::u8, ArduinoStream::Error> const read_byte_result = my_stream.readByte();
    if (read_byte_result.isOk()) {
        kf::u8 const my_byte = read_byte_result.ok();// get result value
    } else {
        // same as read_byte_result.isError()

        ArduinoStream::Error const my_byte_error = read_byte_result.error();
    }

    // read into buffer

    kf::Slice<kf::u8> reading_destination{buffer};
    (void) my_stream.readBuffer(reading_destination);// -> kf::Result<kf::Slice<const kf::u8>, ArduinoStream::Error>

    // read packet instance

    (void) my_stream.readPacket<MyPacket>();// -> kf::Result<MyPacket, ArduinoStream::Error>
}
