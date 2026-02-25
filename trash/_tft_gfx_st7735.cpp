#include <kf/drivers/display/SSD1306.hpp>
#include <kf/drivers/display/ST7735.hpp>
#include <kf/gfx.hpp>

#include <Arduino.h>
#include <cmath>

using namespace kf;
using namespace kf::gfx;

#define RED 0b111111'00000'00000
#define BLUE 0b000000'11111'00000
#define GREEN 0b000000'00000'11111

// Цвета в формате RGB565
constexpr u16 COLOR_RED = RED;
constexpr u16 COLOR_GREEN = GREEN;
constexpr u16 COLOR_BLUE = BLUE;
constexpr u16 COLOR_WHITE = 0xFFFF;
constexpr u16 COLOR_BLACK = 0x0000;
constexpr u16 COLOR_YELLOW = RED | GREEN;
constexpr u16 COLOR_CYAN = GREEN | BLUE;
constexpr u16 COLOR_MAGENTA = RED | BLUE;

using P = ST7735::PixelImpl;

static kf::image::StaticImage<P, 16, 16> test_static_image{};

void render(Canvas<P> &canvas, int t) {
    float ft = static_cast<float>(t) * 0.06f;// Медленная анимация

    // Получаем размеры из canvas
    const Pixels centerX = canvas.centerX();
    const Pixels centerY = canvas.centerY();

    // Простая анимация - движущийся круг
    auto circle_x = static_cast<Pixels>(centerX + centerX * sinf(ft));
    auto circle_y = static_cast<Pixels>(centerY + centerY * cosf(ft));
    auto radius = static_cast<Pixels>(9 + 6 * sinf(ft * 2));

    // Цвет меняется со временем
    u16 r = static_cast<u16>(63 * (0.5f + 0.5f * sinf(ft)));
    u16 g = static_cast<u16>(31 * (0.5f + 0.5f * cosf(ft)));
    u16 b = static_cast<u16>(31 * (0.5f + 0.5f * sinf(ft * 1.5f)));
    u16 circle_color = (r << 10) | (b << 5) | g;

    canvas.setForeground(u16(COLOR_CYAN ^ circle_color));
    canvas.rect(0, 0, canvas.maxX(), canvas.maxY(), false);

    canvas.setForeground(circle_color);
    canvas.circle(circle_x, circle_y, radius, true);

    canvas.setForeground(COLOR_WHITE);
    canvas.circle(circle_x, circle_y, 20, false);

    canvas.setForeground(COLOR_MAGENTA);
    canvas.line(centerX, centerY, circle_x, circle_y);

    canvas.image(circle_x, circle_y, test_static_image);
}

void testOrientation(ST7735 &display) {
    image::DynamicImage<P> display_image{display.image()};
    Canvas<P> canvas(display_image, fonts::gyver_5x7_en);

    const Pixels width = canvas.width();
    const Pixels height = canvas.height();
    char buffer[32];

    auto [a, b] = canvas.split<2>({1, 2}, true);

    auto [c, d] = b.split<2>({2, 3}, false);

    const auto frames_total = 1000;
    const auto start = millis();// Capture the start time at the beginning of the frame loop.

    for (int t = 0; t < frames_total; t += 1) {
        canvas.fill();

        render(a, t / 2);
        render(c, t * 2);
        render(d, t / 10);

        // Display text
        canvas.setForeground(COLOR_WHITE);
        snprintf(buffer, sizeof(buffer), "Frame: \xF3%d\x80\nSize: \xB1\xF2%dx%d", t, width, height);
        canvas.text(5, 5, buffer);

        display.send();
        delay(1);// Control the frame rate
    }

    const auto end = millis();// Capture the end time after all frames are processed.

    const auto duration_ms = end - start;                                     // Total time taken in milliseconds.
    const float ms_per_frame = static_cast<float>(duration_ms) / frames_total;// Correct calculation for avg ms per frame.
    const float fps = 1000 / ms_per_frame;
    Serial.printf("%d frames: FPS: %f (%.6f ms per frame)\n", frames_total, fps, ms_per_frame);// Print the average time per frame.
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting");

    static ST7735::Config display_config{
        GPIO_NUM_5,
        GPIO_NUM_2,
        GPIO_NUM_15,
        27000000,
        ST7735::Orientation::Normal,
    };

    static ST7735 display{display_config, SPI};

    (void) display.init();

    for (auto i = 0; i < test_static_image.buffer().size(); i += 1) {
        test_static_image.buffer()[i] = kf::gfx::Palette<P>::getAnsiColor(static_cast<kf::gfx::Palette<P>::Ansi>(i));
    }

    for (int i = 0; i < 100; i += 1) {
        for (auto o: {
                 ST7735::Orientation::Normal,
                 ST7735::Orientation::ClockWise,
                 ST7735::Orientation::Flip,
                 ST7735::Orientation::CounterClockWise,
             }) {

            display.setOrientation(o);
            testOrientation(display);
        }
    }
}

void loop() {}