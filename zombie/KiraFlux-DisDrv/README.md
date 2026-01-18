# KiraFlux-DisDrv

Display driver library for microcontrollers with a unified interface.

## Drivers

* **SSD1306**: monochrome OLED, 128×64 pixels, I2C
* **ST7735**: color TFT, 128×160 pixels, 16-bit color (RGB565), SPI

## Features

* **Unified API**: base class `DisplayDriver` with methods `init()`, `send()`, `width()`, `height()`, `setOrientation()`
* **CRTP**: static polymorphism via template `DisplayDriver<Impl, ...>`
* **Buffer access**: method `buffer()` returns `kf::slice<BufferItem>` for data manipulation
* **Orientation**: support for reflections and rotations via `setOrientation()`

## Quick Start

### Installation

```ini
lib_deps =
    https://github.com/KiraFlux/KiraFlux-DisDrv.git
    https://github.com/KiraFlux/KiraFlux-ToolBox.git
```

### SSD1306

```cpp
#include <Wire.h>
#include <kf/SSD1306.hpp>

kf::SSD1306 oled{0x3C};

void setup() {
    Wire.begin();
    oled.init();

    auto buf = oled.buffer();
    for (usize i = 0; i < buf.size(); ++i) {
        buf.data()[i] = 0xAA;
    }

    oled.send();
}
```

### ST7735

```cpp
#include <SPI.h>
#include <kf/ST7735.hpp>

SPIClass hspi{HSPI};
kf::ST7735::Settings tft_settings{15, 2, 4};
kf::ST7735 tft{tft_settings, hspi};

void setup() {
    tft.init();

    auto buf = tft.buffer();
    for (usize i = 0; i < buf.size(); ++i) {
        buf.data()[i] = 0xF800;
    }

    tft.send();
}
```

## API

### DisplayDriver

```cpp
bool init();
u8 width() const;
u8 height() const;
void send() const;
void setOrientation(Orientation orientation);
kf::slice<BufferItem> buffer();
u8 maxX() const;
u8 maxY() const;

enum class Orientation : u8 {
    Normal = 0,
    MirrorX = 1,
    MirrorY = 2,
    Flip = 3,
    ClockWise = 4,
    CounterClockWise = 5
};
```

### SSD1306

```cpp
explicit SSD1306(u8 address = 0x3C);
void setContrast(u8 value) const;
void setPower(bool on);
void invert(bool invert);
```

### ST7735

```cpp
struct Settings {
    u8 pin_spi_slave_select;
    u8 pin_data_command;
    u8 pin_reset;
    u32 spi_frequency;
    Orientation orientation;
};
explicit ST7735(const Settings &settings, SPIClass &spi_instance);
```

## Buffer Format

* **SSD1306**: `slice<u8>` with a size of 1024 bytes. Bits in each byte represent vertical pixels.
* **ST7735**: `slice<u16>` with a size of 128×160 elements. RGB565 format.

## License

[MIT License](LICENSE)