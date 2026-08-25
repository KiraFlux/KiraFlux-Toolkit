// KiraFlux-Toolkit Example 'bus/iic-scanner'

#include <kf/bus/I2C.hpp>
#include <kf/gpio.hpp>
#include <kf/main.hpp>
#include <kf/rtos/Task.hpp>

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: bus/iic-scanner");

    // --- Bus configuration ---

    // Configuration for the IIC bus (uses default Wire).
    // none means "use Wire's default".
    bus::I2C::Config const bus_config{
        .gpio_num_sda = some(gpio::G21),
        .gpio_num_scl = some(gpio::G22),
        .clock_hz = some<usize>(100'000),
        .buffer_size = none,
        .timeout_ms = none,
    };

    // Bus instance (must outlive any nodes created from it).
    bus::I2C i2c_bus{bus_config, 0};

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
        bus::I2C::Node::Config node_config{
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