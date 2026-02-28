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

#include <Arduino.h>

#include <kf/drivers/display/ST7735.hpp>
#include <kf/gfx.hpp>

// Pixel format used by the display
using P = kf::ST7735::PixelImpl;

// Predefined colors
constexpr P::ColorType red = P::fromRgb(0xFF, 0x00, 0x00);
constexpr P::ColorType green = P::fromRgb(0x00, 0xFF, 0x00);
constexpr P::ColorType blue = P::fromRgb(0x00, 0x00, 0xFF);
constexpr P::ColorType black = P::fromRgb(0x00, 0x00, 0x00);
constexpr P::ColorType white = P::fromRgb(0xFF, 0xFF, 0xFF);

void demo(kf::ST7735 &display, const char *orientation_name) {
    // Wrap the display's framebuffer (ViewportImage) into a Canvas.
    // display.image() returns a ViewportImage that automatically handles orientation.
    kf::gfx::Canvas<P> canvas{kf::image::DynamicImage<P>{display.image()}};
    canvas.setFont(kf::gfx::fonts::gyver_5x7_en);

    const auto step_time_ms{1000};

    // --- Fill screen with solid colors ---
    canvas.setBackground(red);
    canvas.fill();
    display.send();// transfer framebuffer to hardware
    delay(step_time_ms);

    canvas.setBackground(green);
    canvas.fill();
    display.send();
    delay(step_time_ms);

    canvas.setBackground(blue);
    canvas.fill();
    display.send();
    delay(step_time_ms);

    // --- Draw diagonal lines ---
    canvas.setBackground(white);
    canvas.fill();
    for (int i = 0; i < canvas.width(); i += 10) {
        canvas.setForeground(red);
        canvas.line(0, 0, i, canvas.maxY());
        canvas.setForeground(blue);
        canvas.line(canvas.maxX(), 0, canvas.maxX() - i, canvas.maxY());
    }
    display.send();
    delay(2 * step_time_ms);

    // --- Show current orientation name ---
    canvas.setBackground(black);
    canvas.fill();
    canvas.setForeground(white);
    canvas.text(10, 10, orientation_name);
    display.send();
    delay(2 * step_time_ms);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("ST7735 Driver Demo");

    // Configuration must live as long as the display (static).
    static kf::ST7735::Config config{
        GPIO_NUM_5, // CS
        GPIO_NUM_2, // DC
        GPIO_NUM_15,// RESET
        27'000'000, // SPI frequency
        kf::ST7735::Orientation::Normal,
    };

    // Driver instance references config and SPI bus.
    static kf::ST7735 display{config, SPI};

    if (not display.init()) {
        Serial.println("Display init failed!");
        return;
    }

    // Array of orientation names for display.
    const char *orient_names[] = {"Normal", "Mirror X", "Mirror Y", "Flip", "Clock Wise", "Counter Clock Wise"};

    // Test all orientations.
    for (auto o: {
             kf::ST7735::Orientation::Normal,
             kf::ST7735::Orientation::MirrorX,
             kf::ST7735::Orientation::MirrorY,
             kf::ST7735::Orientation::Flip,
             kf::ST7735::Orientation::ClockWise,
             kf::ST7735::Orientation::CounterClockWise,
         }) {
        display.orientation(o);// change hardware orientation
        demo(display, orient_names[static_cast<int>(o)]);
    }

    Serial.println("Demo finished. Restarting...");
    delay(5000);
    ESP.restart();
}

void loop() {}