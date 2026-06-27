// IIC bus scanner example using KiraFlux IIC abstraction
// Scans all I2C addresses and reports devices that acknowledge.

#include <Arduino.h>
#include <kf/bus/iic/ArduinoIIC.hpp>

using kf::bus::iic::ArduinoIIC;

// Configuration for the IIC bus (uses default Wire)
auto bus_config{
    ArduinoIIC::Config::create(
        // 0 means Wire's default value
        0,// clock
        0,// timeout
        0,// buffer size

        // GPIO_NUM_NC or -1 means Wire's default pins
        21,
        22),
};

// Bus instance (must outlive nodes)
ArduinoIIC bus{bus_config, Wire};

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("IIC Bus Scanner");

    // Initialize the bus
    auto initRes = bus.init();
    if (initRes.isError()) {
        Serial.print("Bus init failed: ");
        switch (initRes.error()) {
            case ArduinoIIC::Error::ClockConfigFailed:
                Serial.println("Clock config failed");
                break;
            case ArduinoIIC::Error::BufferSizeConfigFailed:
                Serial.println("Buffer size config failed");
                break;
            case ArduinoIIC::Error::PinConfigFailed:
                Serial.println("Pin config failed");
                break;
            case ArduinoIIC::Error::BeginFailed:
                Serial.println("Wire.begin() failed");
                break;
            default:
                Serial.println("Unknown error");
        }
        while (true) { delay(1000); }
    }
    Serial.println("Bus initialized");

    // Scan addresses 1..127
    for (uint8_t addr = 1; addr < 0x80; addr += 1) {
        // Create node for this address
        ArduinoIIC::Node::Config node_config{
            .address = addr,
        };

        auto node = bus.createNode(node_config);

        // Try to read a single byte (most devices will NACK if no response)
        auto result = node.readByte();
        if (result.isOk()) {
            // Got a byte – device responded
            Serial.print("Device found at 0x");
            if (addr < 0x10) Serial.print('0');
            Serial.println(addr, HEX);
        } else {
            auto err = result.error();
            if (err == ArduinoIIC::Error::AddressNack) {
                // Normal – no device
            } else if (err == ArduinoIIC::Error::Timeout) {
                Serial.print("Timeout at 0x");
                if (addr < 0x10) Serial.print('0');
                Serial.println(addr, HEX);
            } else {
                // Other error – report
                Serial.print("Error 0x");
                if (addr < 0x10) Serial.print('0');
                Serial.print(addr, HEX);
                Serial.print(": ");
                Serial.println(static_cast<int>(err));
            }
        }
        // Small delay between scans
        delay(5);
    }

    Serial.println("Scan done.");
}

void loop() {}