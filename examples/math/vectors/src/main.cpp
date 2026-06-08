// KiraFlux-Toolkit Demo 'vectors'
#include <Arduino.h>

#include <kf/math/Vector2.hpp>
#include <kf/math/Vector3.hpp>

using namespace kf::math;

void setup() {
    Serial.begin(115200);
    {
        Vector2<float> a1{3.0f, 4.0f};
        Serial.println(a1.length());// 5.0

        Vector2<float> a2{.x = 12.0f, .y = 5.0f};
        Serial.println(a2.length());// 13.0

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
        Vector3<kf::u8> b1{1, 2, 3};
        Vector3<kf::u8> b2{.x = 4, .y = 5, .z = 6};
        auto zero{Vector3<double>::zero()};

        auto sum = b1 + b2;
        auto sub = b1 - b2;
        auto mul_by_scalar = b1 * 10.f;
        auto div_by_scalar = b1 / 10.f;
        auto div_by_scalar_checked_some = b1.divChecked(1);// Option(Some)
        auto div_by_scalar_checked_none = b1.divChecked(0);// Option(None)
        auto scalar_mul = b1.dot(b2);
        auto cross_mul = b1.cross(b2);
        auto normalized = b1.normalized();// None if a1.length() is 0
    }

    // Vector2i = Vector2<i32>
    // Vector2f = Vector2<f32>
}

void loop() {}
