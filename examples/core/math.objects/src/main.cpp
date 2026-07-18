// KiraFlux-Toolkit Demo 'vectors'

#include <kf/main.hpp>
#include <kf/math.hpp>

using namespace kf::math;

void kf::main(kf::Init &) {

    // vector 2d
    {
        Vector2<float> a1{.x = 3.0f, .y = 4.0f};// aggregate initialization
        const auto l1 = a1.length();            // len: 5.0
        auto zero = Vector2f::zero();

        const auto a2 = Vector2<float>::create(12, 5.0);// create with shortcut for static casts
        const auto l2 = a2.length();                    // len: 13.0

        auto sum = a1 + a2;
        auto sub = a1 - a2;
        auto mul_by_scalar = a1 * 10.f;
        auto div_by_scalar = a1 / 10.f;
        auto div_by_scalar_checked_some = a1.divChecked(1);// Option(Some)
        auto div_by_scalar_checked_none = a1.divChecked(0);// Option(None)
        auto scalar_mul = a1.dot(a2);
        auto normalized = a1.normalized();// None if a1.length() is 0

        // Vector2i = Vector2<i32>
        // Vector2f = Vector2<f32>
    }

    // vector 3d
    {

        // construct
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

    // quaternion
    {

        // construct

        Quaternion<float> q1{
            .x = 0,
            .y = 0,
            .z = 0,
            .w = 1,
        };

        const auto q2 = Quaternion<double>::create(1, 2.0f, 3.0, 0);

        const auto i = Quaternion<float>::identity();

        const auto c = q1.conjugate();
        const auto v = q1.inverse();
        const auto n = q1.normalized();

        q1.normalize();// applied to self

        const auto l = q1.length();
        const auto l2 = q1.lengthSquared();
        const auto rotated = q2.rotate(Vector3f::create(1, 0, 0));
        const auto e = q2.toEulers();
    }
}
