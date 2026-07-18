// IIC bus scanner example using KiraFlux IIC abstraction
// Scans all I2C addresses and reports devices that acknowledge.

#include <kf/String.hpp>
#include <kf/main.hpp>

#include <kf/arduino/ArduinoIIC.hpp>

using kf::arduino::ArduinoIIC;

// Configuration for the IIC bus (uses default Wire)
ArduinoIIC::Config bus_config{

    // GPIO_NUM_NC or -1 means Wire's default pins
    .gpio_num_sda = static_cast<kf::u8>(21),
    .gpio_num_scl = static_cast<kf::u8>(22),
    
    // 0 means Wire's default value
    .buffer_size = 0,
    .clock_hz = 0,
    .timeout = 0,
};

// Bus instance (must outlive nodes)
ArduinoIIC i2c_bus{bus_config, Wire};

const char *stringFromError(ArduinoIIC::Error e) {
    switch (e) {
        case ArduinoIIC::Error::ClockConfigFailed: return "Clock config failed";
        case ArduinoIIC::Error::BufferSizeConfigFailed: return "Buffer size config failed";
        case ArduinoIIC::Error::PinConfigFailed: return "Pin config failed";
        case ArduinoIIC::Error::BeginFailed: return "Wire.begin() failed";
        case ArduinoIIC::Error::AddressNack: return "Address NACK";
        case ArduinoIIC::Error::DataNack: return "Data NACK";
        case ArduinoIIC::Error::Timeout: return "Timeout";
        case ArduinoIIC::Error::BufferTooLong: return "Buffer too long";
        case ArduinoIIC::Error::IncompletePacket: return "Incomplete packet";
        default: return "Unknown error";
    }
}

void kf::main(kf::Init &init) {
    init.logger.info("IIC Bus Scanner");
    char str_buffer[128];
    kf::String str{{str_buffer}};

    // Initialize the bus
    if (const auto result = i2c_bus.init(); result.isError()) {
        str.format("Bus init failed: {}", stringFromError(result.error()));
        init.logger.error(str.view());
        return;
    }

    init.logger.info("Bus initialized successfully");

    // Scan addresses 1..127
    for (kf::u8 address = 1; address < 0x80; address += 1) {
        // Create node for this address
        ArduinoIIC::Node::Config node_config{
            .address = address,
        };

        auto node = i2c_bus.createNode(node_config);

        // Try to write empty slice - device acknowledges with ACK if present.
        auto result = node.writeBuffer({});
        if (result.isOk()) {
            // Device responded with ACK
            str.format("Device found at {}", address);
            init.logger.info(str.view());
        } else {
            // log errors
            str.format("At {}: {}", address, stringFromError(result.error()));
            init.logger.error(str.view());
        }

        // Small delay between scans
        delay(5);
    }

    init.logger.info("Scan done.");
}