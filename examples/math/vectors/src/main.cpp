// KiraFlux-Toolkit Demo 'vectors'
#include <Arduino.h>

#include <kf/math.hpp>

using namespace kf::math;

void setup() {
    Serial.begin(115200);
    {
        Vector2<float> a1{.x = 3.0f, .y = 4.0f};// aggregate initialization
        Serial.println(a1.length());            // len: 5.0

        const auto a2 = Vector2<float>::create(12, 5.0);// create with shortcut for static casts
        Serial.println(a2.length());                    // len: 13.0

        auto sum = a1 + a2;
        auto sub = a1 - a2;
        auto mul_by_scalar = a1 * 10.f;
        auto div_by_scalar = a1 / 10.f;
        auto div_by_scalar_checked_some = a1.divChecked(1);// Option(Some)
        auto div_by_scalar_checked_none = a1.divChecked(0);// Option(None)
        auto scalar_mul = a1.dot(a2);
        auto normalized = a1.normalized();// None if a1.length() is 0
    }
    {
        Vector3<kf::u8> b1{.x = 1, .y = 2, .z = 3};
        const auto b2 = Vector3<kf::u8>::create(4, 5.0f, 6.0);
        auto zero{Vector3<double>::zero()};

        auto sum = b1 + b2;
        auto sub = b1 - b2;
        auto mul_by_scalar = b1 * 10.f;
        auto div_by_scalar = b1 / 10.f;
        auto div_by_scalar_checked_some = b1.divChecked(1);// Option(Some)
        auto div_by_scalar_checked_none = b1.divChecked(0);// Option(None)
        auto scalar_mul = b1.dot(b2);
        auto cross_mul = b1.cross(b2);
        auto normalized = b1.normalized();// None if b1.length() is 0
    }

    // Vector2i = Vector2<i32>
    // Vector2f = Vector2<f32>
}

void loop() {}
