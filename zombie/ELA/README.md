# ELA (Embedded Linear Algebra)

> Minimalist linear algebra library for embedded systems with safe operations

## Key Features

- Header-only 2D/3D vector implementations for embedded systems
- Type-safe templates with `float`, `int`, and `double` support
- Safe operations with `rs::Option` error handling
- Zero-allocation design with `constexpr` and `noexcept` optimizations
- Comprehensive vector operations (dot product, cross product, normalization)

## Quick Start

### Installation (PlatformIO)

Add to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/KiraFlux/ELA.git
```

### Minimal Example

```cpp
#include <ela/vec2.hpp>
#include <ela/vec3.hpp>

void setup() {
    // 2D vector operations
    ela::vec2f position{3.0f, 4.0f};
    if (auto dir = position.normalized()) {
        // Safe usage of normalized vector
    }
    
    // 3D physics calculation
    ela::vec3f force{1.5f, 2.0f, 0.5f};
    ela::vec3f lever{0.0f, 1.0f, 2.0f};
    ela::vec3f torque = force.cross(lever);
}
```

## API Reference

### `vec2<T>`

```cpp
// Constructors
vec2() noexcept;                 // Zero-initialized
vec2(T x, T y) noexcept;         // Component initialization

// Operators
vec2 operator+(const vec2& rhs) const noexcept;
vec2 operator-(const vec2& rhs) const noexcept;
vec2 operator*(T scalar) const noexcept;
vec2 operator/(T scalar) const noexcept;       // Unsafe division
vec2& operator+=(const vec2& rhs) noexcept;
vec2& operator-=(const vec2& rhs) noexcept;

// Methods
T length() const noexcept;                     // Euclidean length
rs::Option<vec2> normalized() const noexcept;  // Safe normalization
T dot(const vec2& other) const noexcept;       // Dot product
bool isZero() const noexcept;                  // Zero vector check
rs::Option<vec2> divChecked(T scalar) const noexcept;  // Safe division

// Type aliases
using vec2f = vec2<float>;
using vec2i = vec2<int>;
using vec2d = vec2<double>;
```

### `vec3<T>`

```cpp
// Constructors
vec3() noexcept;                 // Zero-initialized
vec3(T x, T y, T z) noexcept;    // Component initialization

// Operators
vec3 operator+(const vec3& rhs) const noexcept;
vec3 operator-(const vec3& rhs) const noexcept;
vec3 operator*(T scalar) const noexcept;
vec3 operator/(T scalar) const noexcept;       // Unsafe division
vec3& operator+=(const vec3& rhs) noexcept;
vec3& operator-=(const vec3& rhs) noexcept;

// Methods
T length() const noexcept;                     // Euclidean length
rs::Option<vec3> normalized() const noexcept;  // Safe normalization
T dot(const vec3& other) const noexcept;       // Dot product
vec3 cross(const vec3& other) const noexcept;  // Cross product
bool isZero() const noexcept;                  // Zero vector check
rs::Option<vec3> divChecked(T scalar) const noexcept;  // Safe division

// Type aliases
using vec3f = vec3<float>;
using vec3i = vec3<int>;
using vec3d = vec3<double>;
```

## Usage Examples

### Sensor Data Processing

```cpp
#include <ela/vec3.hpp>

struct IMUData {
    ela::vec3f acceleration;
    ela::vec3f gyro;
};

void process_sensor(IMUData& data) {
    // Safe normalization
    if (auto norm_accel = data.acceleration.normalized()) {
        // Use normalized vector
    }
    
    // Remove gravity component (assumes z-up)
    constexpr ela::vec3f gravity{0, 0, 9.8f};
    ela::vec3f linear_accel = data.acceleration - gravity;
    
    // Integration with delta time
    float dt = 0.01f;
    data.gyro = data.gyro * dt;
}
```

### 2D Navigation

```cpp
#include <ela/vec2.hpp>

class RobotNavigator {
    ela::vec2f position;
    ela::vec2f target;
    const float speed = 0.5f;
    
public:
    void update(float dt) {
        ela::vec2f direction = target - position;
        
        // Safe direction handling
        if (auto normalized_dir = direction.normalized()) {
            position += *normalized_dir * speed * dt;
        }
        
        // Check arrival
        if (position.isZero()) {
            // Handle arrival
        }
    }
};
```

## Integration

### Supported Platforms

- ESP32 (Tested)
- Any C++11 compatible embedded platform

### PlatformIO Configuration

```ini
lib_deps =
    https://github.com/KiraFlux/ELA.git
```

## Limitations

- No matrix operations support
- No quaternion implementation
- Requires Rustify for Option type
- Limited to 2D/3D vectors only

## License

[MIT License](LICENSE)