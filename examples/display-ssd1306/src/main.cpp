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

#include <kf/drivers/display/SSD1306.hpp>
#include <kf/gfx.hpp>

// Pixel format used by the display (monochrome)
using P = kf::SSD1306::PixelImpl;// = pixel::MonochromePixel
using Color = P::ColorType;      // = bool

constexpr Color ON = true;
constexpr Color OFF = false;

void demo(kf::SSD1306 &display, const char *orientation_name) {
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

    // I2C bus (use default Wire)
    // Wire.begin(); // may already be called by the driver, but call here to be safe
    Wire.begin();

    // Display configuration
    static kf::SSD1306::Config config{
        400'000,// I2C clock frequency (400 kHz typical)
        0x3C    // I2C address
    };

    // Driver instance references config and Wire.
    static kf::SSD1306 display{config, Wire};

    if (not display.init()) {
        Serial.println("Display init failed!");
        return;
    }

    // Array of orientation names (only those supported by SSD1306)
    const char *orient_names[] = {"Normal", "Mirror X", "Mirror Y"};

    // Test only supported orientations.
    for (auto o: {
             kf::SSD1306::Orientation::Normal,
             kf::SSD1306::Orientation::MirrorX,
             kf::SSD1306::Orientation::MirrorY}) {
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