# KiraFlux Graphics Library

## Components

### `DynamicImage<Format>`

Represents a display region with runtime dimensions.

```cpp
// Creation with validation
static Result<DynamicImage, Error> create(
    BufferType* buffer, 
    Pixel stride, 
    Pixel width, 
    Pixel height,
    Pixel offset_x, 
    Pixel offset_y
)

// Default constructor - invalid view
DynamicImage()

// Unsafe constructor without validation
explicit DynamicImage(
    BufferType* buffer, 
    Pixel stride, 
    Pixel width, 
    Pixel height,
    Pixel offset_x, 
    Pixel offset_y
)

// Create validated sub-region
Result<DynamicImage, Error> sub(
    Pixel sub_width, 
    Pixel sub_height,
    Pixel sub_offset_x, 
    Pixel sub_offset_y
) const

// Create sub-region without validation
DynamicImage subUnchecked(
    Pixel sub_width, 
    Pixel sub_height,
    Pixel sub_offset_x, 
    Pixel sub_offset_y
)

// Check if X coordinate is within bounds
bool isInsideX(Pixel x) const

// Check if Y coordinate is within bounds
bool isInsideY(Pixel y) const

// Check if view references valid buffer
bool isValid() const

// Set single pixel color
void setPixel(Pixel x, Pixel y, ColorType color) const

// Fill entire region with solid color
void fill(ColorType color) const
```

### `Canvas<Format>`

Drawing context with graphics primitives.

```cpp
// Constructor with existing frame
explicit Canvas(
    const DynamicImage<Format>& frame,
    const Font& font = Font::blank(),
    ColorType foreground = Traits::foreground_default,
    ColorType background = Traits::background_default
)

// Default constructor - invalid canvas
explicit Canvas()

// Create validated sub-canvas
Result<Canvas, typename DynamicImage<Format>::Error> sub(
    Pixel width, Pixel height,
    Pixel offset_x, Pixel offset_y
)

// Create sub-canvas without validation
Canvas subUnchecked(
    Pixel width, Pixel height,
    Pixel offset_x, Pixel offset_y
)

// Split canvas into weighted sub-canvases
template<usize N> array<Canvas, N> split(array<usize, N> weights, bool horizontal = true)
```

#### Attributes

```cpp
Pixel width() const
Pixel height() const
Pixel maxX() const
Pixel maxY() const
Pixel centerX() const
Pixel centerY() const
Pixel tabWidth() const
```

#### Control

```cpp
void setFont(const Font& font)
void setBackground(ColorType color)
void setForeground(ColorType color)
void swapColors()
void setAutoNextLine(bool enable)
```

#### Drawing API

```cpp
void fill() const
void dot(Pixel x, Pixel y) const
void line(Pixel x0, Pixel y0, Pixel x1, Pixel y1) const
void rect(Pixel x0, Pixel y0, Pixel x1, Pixel y1, bool fill)
void circle(Pixel cx, Pixel cy, Pixel r, bool fill)
void text(Pixel start_x, Pixel start_y, const char* text)
template<Pixel W, Pixel H> void image(Pixel x, Pixel y, const StaticImage<Format, W, H>& image)
```

### `StaticImage<Format, W, H>`

Predefined image with compile-time dimensions.

```cpp
// Default constructor deleted
StaticImage() = delete

Pixel width() const
Pixel height() const
```

### `Font`

Monospaced font with glyph height up to 8 pixels.

```cpp
// Get blank font instance
static const Font& blank()

// Get total width including spacing
u8 widthTotal() const

// Get total height including spacing
u8 heightTotal() const

// Get pointer to glyph data
const u8 *getGlyph(char c) const
```

## Error Types

### `DynamicImage::Error`

```cpp
enum class Error : u8 {
    BufferNotInit,    // Buffer pointer is null
    SizeTooSmall,     // Region dimensions < 1 pixel
    SizeTooLarge,     // Sub-region exceeds parent bounds
    OffsetOutOfBounds // Offset falls outside parent region
}
```

## Text Formatting Codes

- `\x80` - Reset to foreground color
- `\x81` - Use background color (inverse)
- `\x82` - Start from horizontal center
- `\x83` - Set custom color (followed by color bytes)
- `\n` - New line
- `\t` - Tab (4 character widths)

## Dependencies

- **KiraFlux-ToolBox** (required)
- **C++11** (minimum), **C++17** (recommended)

# LICENCE

[MIT](LICENSE)