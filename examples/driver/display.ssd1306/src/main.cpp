// KiraFlux-Toolkit Example 'driver/display.ssd1306'

#include <kf/bus/I2C.hpp>
#include <kf/driver/display/SSD1306.hpp>
#include <kf/gfx/Canvas.hpp>
#include <kf/gfx/fonts/gyver_5x7.hpp>
#include <kf/image/DynamicImage.hpp>
#include <kf/main.hpp>

using kf::driver::display::Orientation;
using kf::driver::display::SSD1306;

// Pixel format: monochrome (1‑bit)
using P = SSD1306::PixelImpl;
using Color = P::ColorType;// bool

constexpr Color ON = true;
constexpr Color OFF = false;

// --- Helper: run a demo sequence for a given orientation ---

void runDemo(kf::Init &init, SSD1306 &display, char const *orientation_name) {
    // Wrap the display framebuffer into a Canvas
    kf::gfx::Canvas<P> canvas{kf::image::DynamicImage<P>{display.image()}};
    canvas.font(kf::gfx::fonts::gyver_5x7_en);

    auto const step_time_ms = 1000;

    // Fill screen with ON (all pixels white)
    canvas.background(ON);
    canvas.fill();
    (void) display.send();
    delay(step_time_ms);

    // Fill screen with OFF (clear)
    canvas.background(OFF);
    canvas.fill();
    (void) display.send();
    delay(step_time_ms);

    // Draw diagonal lines
    canvas.background(OFF);
    canvas.fill();
    canvas.foreground(ON);
    for (int i = 0; i < canvas.width(); i += 10) {
        canvas.line(0, 0, i, canvas.maxY());
        canvas.line(canvas.maxX(), 0, canvas.maxX() - i, canvas.maxY());
    }
    (void) display.send();
    delay(2 * step_time_ms);

    // Show orientation name
    canvas.background(OFF);
    canvas.fill();
    canvas.foreground(ON);
    canvas.text(10, 10, orientation_name);
    (void) display.send();
    delay(2 * step_time_ms);

    init.logger.info("Demo for {} completed", orientation_name);
}

// --- Main application ---

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: driver/display.ssd1306");

    // --- I2C bus configuration ---

    // 0xFF (-1) means "use Wire's default GPIOs".
    // 0 means "use Wire's default value" for clock, timeout, buffer size.
    static bus::I2C::Config bus_config{
        .gpio_num_sda = 0xFF,
        .gpio_num_scl = 0xFF,
        .buffer_size = 0,
        .clock_hz = 400'000,// 400 kHz (typical for I2C)
        .timeout = 0,
    };

    bus::I2C i2c_bus{
        bus_config,// by ref
        0,         // i2c bus number
    };

    // Initialize the I2C bus
    auto init_result = i2c_bus.init();
    if (init_result.isError()) {
        init.logger.error("I2C bus init failed: {}", init_result.error());
        return;
    }
    init.logger.info("I2C bus initialized");

    // --- Display configuration ---

    static bus::I2C::Node::Config node_config{
        .address = 0x3C,// default SSD1306 address
    };

    // Create the display driver instance (static to outlive the function)
    static SSD1306 display{std::move(i2c_bus.createNode(node_config))};

    // Initialize the display
    auto display_init = display.init();
    if (display_init.isError()) {
        init.logger.error("Display init failed: {}", display_init.error());
        return;
    }
    init.logger.info("Display initialized");

    // --- Test supported orientations ---

    // SSD1306 supports: Normal, MirrorX, MirrorY
    char const *orientation_names[] = {"Normal", "Mirror X", "Mirror Y"};
    Orientation orientations[] = {
        Orientation::Normal,
        Orientation::MirrorX,
        Orientation::MirrorY,
    };

    for (usize i = 0; i < 3; i += 1) {
        auto orient_result = display.orientation(orientations[i]);
        if (orient_result.isOk()) {
            runDemo(init, display, orientation_names[i]);
        } else {
            init.logger.error("Orientation '{}' set error: {}", orientation_names[i], orient_result.error());
        }
    }

    init.logger.info("Demo completed.");
}