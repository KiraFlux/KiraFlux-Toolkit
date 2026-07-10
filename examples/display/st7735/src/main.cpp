// ST7735 Driver Demo
//
// Focus: display driver initialization, orientation, framebuffer transfer.
// Graphics are drawn via kf::gfx::Canvas (using display's image buffer) for clarity.
//
// Connections (ESP32):
// - TFT_CS   -> GPIO5
// - TFT_DC   -> GPIO2
// - TFT_RST  -> GPIO15
// - TFT_MOSI -> GPIO23 (default SPI MOSI)
// - TFT_SCLK -> GPIO18 (default SPI SCK)
// - VCC      -> 3.3V
// - GND      -> GND

#include <kf/gfx/Canvas.hpp>
#include <kf/gfx/Palette.hpp>
#include <kf/gfx/fonts/gyver_5x7.hpp>
#include <kf/image/DynamicImage.hpp>

#include <Arduino.h>
#include <kf/bus/spi/ArduinoSPI.hpp>
#include <kf/drivers/display/Orientation.hpp>
#include <kf/drivers/display/ST7735.hpp>
#include <kf/gpio/ArduinoGPIO.hpp>

using kf::drivers::display::Orientation;

using kf::bus::spi::ArduinoSPI;
using kf::gpio::ArduinoGPIO;
using ST7735 = kf::drivers::display::ST7735<ArduinoSPI::Node, ArduinoGPIO::DigitalOutput>;

using P = ST7735::PixelImpl;        // Pixel format used by the display
using Palette = kf::gfx::Palette<P>;// Palette for this pixel format

// Predefined colors
constexpr P::ColorType red = P::fromRgb(0xFF, 0x00, 0x00);
constexpr P::ColorType green = P::fromRgb(0x00, 0xFF, 0x00);
constexpr P::ColorType blue = P::fromRgb(0x00, 0x00, 0xFF);
constexpr P::ColorType black = Palette::black;
constexpr P::ColorType white = Palette::white;

void demo(ST7735 &display, const char *orientation_name) {
    // Wrap the display's framebuffer (ViewportImage) into a Canvas.
    // display.image() returns a ViewportImage that automatically handles orientation.
    kf::gfx::Canvas<P> canvas{kf::image::DynamicImage<P>{display.image()}};
    canvas.font(kf::gfx::fonts::gyver_5x7_en);

    const auto step_time_ms{1000};

    // --- Fill screen with solid colors ---
    canvas.background(red);
    canvas.fill();
    (void) display.send();// transfer framebuffer to hardware
    delay(step_time_ms);

    canvas.background(green);
    canvas.fill();
    (void) display.send();
    delay(step_time_ms);

    canvas.background(blue);
    canvas.fill();
    (void) display.send();
    delay(step_time_ms);

    // --- Draw diagonal lines ---
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

    // --- Show current orientation name ---
    canvas.background(black);
    canvas.fill();
    canvas.foreground(white);
    canvas.text(10, 10, orientation_name);
    (void) display.send();
    delay(2 * step_time_ms);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("ST7735 Driver Demo");

    // use defaults
    static ArduinoSPI::Config spi_bus_config{
        // use defaults
        .gpio_num_mosi = static_cast<kf::u8>(GPIO_NUM_NC),
        .gpio_num_miso = static_cast<kf::u8>(GPIO_NUM_NC),
        .gpio_num_sck = static_cast<kf::u8>(GPIO_NUM_NC),
    };

    static ArduinoSPI spi_bus{
        spi_bus_config,
        SPI,
    };

    static auto spi_node_config{
        ArduinoSPI::Node::Config::create(
            // CS
            GPIO_NUM_5,
            // SPI frequency
            27'000'000),
    };

    // Configuration must live as long as the display (static).
    static ST7735::Config driver_config{
        .init_orientation = Orientation::Normal,
    };

    // Driver instance references config and SPI bus.
    static ST7735 display{
        driver_config,
        spi_bus.createNode(spi_node_config),
        ArduinoGPIO::DigitalOutput{GPIO_NUM_22},// DC
        ArduinoGPIO::DigitalOutput{GPIO_NUM_17},// RESET
    };

    (void) spi_bus.init();

    if (display.init().isError()) {
        Serial.println("Display init failed!");
        return;
    }

    // Array of orientation names for display.
    const char *orient_names[] = {"Normal", "Mirror X", "Mirror Y", "Flip", "Clock Wise", "Counter Clock Wise"};

    // Test all orientations.
    for (auto o: {
             Orientation::Normal,
             Orientation::MirrorX,
             Orientation::MirrorY,
             Orientation::Flip,
             Orientation::ClockWise,
             Orientation::CounterClockWise,
         }) {
        (void) display.orientation(o);// change hardware orientation
        demo(display, orient_names[static_cast<int>(o)]);
    }

    Serial.println("Demo finished. Restarting...");
    delay(5000);
    ESP.restart();
}

void loop() {}