// SSD1306 OLED Display Demo
//
// Focus: display driver initialization, orientation, framebuffer transfer.
// Graphics are drawn via kf::gfx::Canvas (using display's image buffer) for clarity.
//
// Connections (ESP32):
// - SDA -> GPIO21 (or your I2C SDA pin)
// - SCL -> GPIO22 (or your I2C SCL pin)
// - VCC -> 3.3V
// - GND -> GND
//
// Note: The default I2C address is 0x3C. Change in config if needed.

#include <Arduino.h>
#include <Wire.h>
#include <kf/gfx/Canvas.hpp>
#include <kf/gfx/fonts/gyver_5x7.hpp>
#include <kf/image/DynamicImage.hpp>

// Actually uses in this demo
#include <kf/bus/iic/ArduinoIIC.hpp>
#include <kf/drivers/display/SSD1306.hpp>

using kf::bus::iic::ArduinoIIC;
using kf::drivers::display::Orientation;
using SSD1306 = kf::drivers::display::SSD1306<ArduinoIIC>;

// Pixel format used by the display (monochrome)
using P = SSD1306::PixelImpl;// = pixel::MonochromePixel
using Color = P::ColorType;  // = bool

constexpr Color ON = true;
constexpr Color OFF = false;

void demo(SSD1306 &display, const char *orientation_name) {
    // Wrap the display's framebuffer into a Canvas.
    // display.image() returns a StaticImage<MonochromePixel, 128, 64>.
    kf::gfx::Canvas<P> canvas{kf::image::DynamicImage<P>{display.image()}};
    canvas.font(kf::gfx::fonts::gyver_5x7_en);

    const auto step_time_ms{1000};

    // --- Fill screen with solid ON (all pixels on) ---
    canvas.background(ON);
    canvas.fill();
    (void) display.send();
    delay(step_time_ms);

    // --- Fill screen with OFF (clear) ---
    canvas.background(OFF);
    canvas.fill();
    (void) display.send();
    delay(step_time_ms);

    // --- Draw diagonal lines ---
    canvas.background(OFF);
    canvas.fill();
    canvas.foreground(ON);
    for (int i = 0; i < canvas.width(); i += 10) {
        canvas.line(0, 0, i, canvas.maxY());
        canvas.line(canvas.maxX(), 0, canvas.maxX() - i, canvas.maxY());
    }
    (void) display.send();
    delay(2 * step_time_ms);

    // --- Show current orientation name ---
    canvas.background(OFF);
    canvas.fill();
    canvas.foreground(ON);
    canvas.text(10, 10, orientation_name);
    (void) display.send();
    delay(2 * step_time_ms);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("SSD1306 Driver Demo");

    static ArduinoIIC::Config bus_config{
        ArduinoIIC::Config::create(
            // 0 means Wire's default value
            400'000,// I2C clock frequency (400 kHz typical)
            0,      // timeout
            0,      // buffer size

            // GPIO_NUM_NC or -1 means Wire's default pins
            -1,
            -1),
    };

    ArduinoIIC bus{bus_config, Wire};

    (void) bus.init();

    // Display configuration
    static ArduinoIIC::Node::Config config{
        .address = 0x3C,
    };

    // Driver instance references config and Wire.
    static SSD1306 display{std::move(bus.createNode(config))};

    if (not display.init()) {
        Serial.println("Display init failed!");
        return;
    }

    // Array of orientation names (only those supported by SSD1306)
    const char *orient_names[] = {"Normal", "Mirror X", "Mirror Y"};

    // Test only supported orientations.
    for (auto o: {
             Orientation::Normal,
             Orientation::MirrorX,
             Orientation::MirrorY}) {
        if (display.orientation(o)) {
            demo(display, orient_names[static_cast<int>(o)]);
        } else {
            Serial.print("Orientation ");
            Serial.print(orient_names[static_cast<int>(o)]);
            Serial.println(" not supported, skipping.");
        }
    }

    Serial.println("Demo finished. Restarting...");
    delay(5000);
    ESP.restart();
}

void loop() {}