// KiraFlux-Toolkit Demo 'vectors'
#include <Arduino.h>

#include <kf/math/Vector2.hpp>
#include <kf/math/Vector3.hpp>

using namespace kf::math;

void setup() {
    Serial.begin(115200);

    Vector2<float> a1{3.0f, 4.0f};
    Serial.println(a1.length());// 5.0

    Vector2<float> a2{12.0f, 5.0f};
    Serial.println(a2.length());// 13.0

    auto sum = a1 + a2;
    auto sub = a1 - a2;
    auto mul_by_scalar = a1 * 10.f;
    auto div_by_scalar = a1 / 10.f;
    auto div_by_scalar_checked = a1.divChecked(1); // Option(Some)
    auto div_by_scalar_checked = a1.divChecked(0); // Option(None)
    auto scalar_mul = a1.dot(a2);

    Vector3<kf::u8> b1{1, 2, 3};
    Vector3<kf::u8> b2{4, 5, 6};

    auto sum = b1 + b2;
    auto sub = b1 - b2;
    auto mul_by_scalar = b1 * 10.f;
    auto div_by_scalar = b1 / 10.f;
    auto div_by_scalar_checked = b1.divChecked(1); // Option(Some)
    auto div_by_scalar_checked = b1.divChecked(0); // Option(None)
    auto scalar_mul = b1.dot(b2);
    auto cross_mul = b1.cross(b2);
}

void loop() {}
