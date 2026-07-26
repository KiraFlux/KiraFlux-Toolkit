// KiraFlux-Toolkit Example 'bus/iic-scanner'

#include <kf/bus/I2C.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>

using kf::bus::I2C;

// --- Bus configuration ---

// Configuration for the IIC bus (uses default Wire).
// GPIO_NUM_NC or -1 means "use Wire's default pins".
// A value of 0 for clock, timeout, or buffer size also means "use Wire's default".
I2C::Config bus_config{
    .gpio_num_sda = static_cast<kf::u8>(21),
    .gpio_num_scl = static_cast<kf::u8>(22),
    .buffer_size = 0,
    .clock_hz = 0,
    .timeout = 0,
};

// Bus instance (must outlive any nodes created from it).
I2C i2c_bus{bus_config, 0};

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: bus/iic-scanner");

    // --- Initialize the bus ---

    if (auto const result = i2c_bus.init(); result.isError()) {
        init.logger.error("Bus init failed: {}", result.error());
        return;
    }
    init.logger.info("Bus initialized successfully");

    // --- Scan all I2C addresses ---

    // Address range: 0x01 .. 0x7F (127). Address 0x00 is reserved for general call.
    // For each address, we attempt to write an empty buffer to see if the device ACKs.
    // If the device responds with ACK, it is present on the bus.

    for (kf::u8 address = 1; address < 0x80; address += 1) {
        // Create a node for this address.
        I2C::Node::Config node_config{
            .address = address,
        };
        auto node = i2c_bus.createNode(node_config);

        // Attempt to write an empty buffer. The device acknowledges with ACK if present.
        auto const result = node.writeBuffer({});

        if (result.isOk()) {
            init.logger.info("Device found at {}", address);
        } else {
            // Log the error at debug level (common: AddressNack, Timeout, etc.)
            init.logger.debug("No device at {} (error: {})", address, result.error());
        }

        // Small delay to avoid bus contention or excessive polling.
        rtos::Task::sleep(5);
    }

    // --- Done ---

    init.logger.info("Scan completed.");
}