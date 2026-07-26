// KiraFlux-Toolkit Example 'driver/display.st7735'

#include <kf/bus/SPI.hpp>
#include <kf/driver/display/Orientation.hpp>
#include <kf/driver/display/ST7735.hpp>
#include <kf/gfx/Canvas.hpp>
#include <kf/gfx/Palette.hpp>
#include <kf/gfx/fonts/gyver_5x7.hpp>
#include <kf/gpio.hpp>
#include <kf/image/DynamicImage.hpp>
#include <kf/main.hpp>

using kf::driver::display::Orientation;
using kf::driver::display::ST7735;

// Pixel format: RGB565 (16‑bit)
using P = ST7735::PixelImpl;
using Palette = kf::gfx::Palette<P>;

// Predefined colors (using hardware-specific RGB conversion)
constexpr auto red = P::fromRgb(0xFF, 0x00, 0x00);
constexpr auto green = P::fromRgb(0x00, 0xFF, 0x00);
constexpr auto blue = P::fromRgb(0x00, 0x00, 0xFF);
constexpr auto black = Palette::black;
constexpr auto white = Palette::white;

// --- Helper: run a demo sequence for a given orientation ---

void runDemo(kf::Init &init, ST7735 &display, char const *orientation_name) {
    // Wrap the display framebuffer into a Canvas
    kf::gfx::Canvas<P> canvas{kf::image::DynamicImage<P>{display.image()}};
    canvas.font(kf::gfx::fonts::gyver_5x7_en);

    auto const step_time_ms = 1000;

    // Fill screen with solid colors
    canvas.background(red);
    canvas.fill();
    (void) display.send();
    delay(step_time_ms);

    canvas.background(green);
    canvas.fill();
    (void) display.send();
    delay(step_time_ms);

    canvas.background(blue);
    canvas.fill();
    (void) display.send();
    delay(step_time_ms);

    // Draw diagonal lines on white background
    canvas.background(white);
    canvas.fill();
    for (int i = 0; i < canvas.width(); i += 10) {
        canvas.foreground(red);
        canvas.line(0, 0, i, canvas.maxY());
        canvas.foreground(blue);
        canvas.line(canvas.maxX(), 0, canvas.maxX() - i, canvas.maxY());
    }
    (void) display.send();
    delay(2 * step_time_ms);

    // Show orientation name on black background
    canvas.background(black);
    canvas.fill();
    canvas.foreground(white);
    canvas.text(10, 10, orientation_name);
    (void) display.send();
    delay(2 * step_time_ms);

    init.logger.info("Demo for {} completed", orientation_name);
}

// --- Main application ---

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: driver/display.st7735");

    // --- SPI bus configuration ---

    // Use default SPI pins (GPIO_NUM_NC means "use hardware defaults")
    static bus::SPI::Config spi_bus_config{
        .gpio_num_mosi = static_cast<u8>(GPIO_NUM_NC),
        .gpio_num_miso = static_cast<u8>(GPIO_NUM_NC),
        .gpio_num_sck = static_cast<u8>(GPIO_NUM_NC),
    };

    bus::SPI spi_bus{spi_bus_config};

    // Initialize the SPI bus
    auto init_result = spi_bus.init();
    if (init_result.isError()) {
        init.logger.error("SPI bus init failed");
        return;
    }
    init.logger.info("SPI bus initialized");

    // --- Display configuration ---

    // Driver configuration
    static ST7735::Config driver_config{
        // Node configuration
        .spi_node = {
            .clock_hz = 27'000'000,
            .gpio_num_cs = static_cast<u8>(GPIO_NUM_5),
            .bit_order = bus::SPI::Node::Config::BitOrder::MostSignificant,
            .clock_bits = bus::SPI::Node::Config::ClockBits::None,
        },

        .init_orientation = Orientation::Normal,
    };

    // Driver instance (static: SHOULD NOT lay on stack cuz buffer image is 40KiB)
    static ST7735 display{
        spi_bus,      // used to create node
        driver_config,// by const reference
        gpio::G22,    // DC
        gpio::G17,    // RESET
    };

    // Initialize the display
    auto display_init = display.init();
    if (display_init.isError()) {
        init.logger.error("Display init failed");
        return;
    }
    init.logger.info("Display initialized");

    // --- Test all orientations (6 supported modes) ---

    Orientation orientations[] = {
        Orientation::Normal,
        Orientation::MirrorX,
        Orientation::MirrorY,
        Orientation::Flip,
        Orientation::ClockWise,
        Orientation::CounterClockWise,
    };

    char const *orientation_names[] = {
        "Normal",
        "Mirror X",
        "Mirror Y",
        "Flip",
        "Clock Wise",
        "Counter Clock Wise",
    };

    for (usize i = 0; i < 6; i += 1) {
        auto orient_result = display.orientation(orientations[i]);
        if (orient_result.isOk()) {
            runDemo(init, display, orientation_names[i]);
        } else {
            init.logger.error("Orientation '{}' set error", orientation_names[i]);
        }
    }

    init.logger.info("Demo completed.");
}