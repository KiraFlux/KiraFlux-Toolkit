// KiraFlux-Toolkit Example 'core/math.objects'

#include <kf/main.hpp>
#include <kf/math.hpp>

using namespace kf::math;

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: core/math.objects");

    // --- Vector2 ---

    {
        init.logger.info("--- Vector2 ---");

        // Create vectors using aggregate initialization or static factory
        Vector2f a1{.x = 3.0f, .y = 4.0f};          // aggregate
        Vector2f a2 = Vector2f::create(12.0f, 5.0f);// factory (auto casts)

        init.logger.info("a1: ({}, {}), len: {}", a1.x, a1.y, a1.length());
        init.logger.info("a2: ({}, {}), len: {}", a2.x, a2.y, a2.length());

        // Basic arithmetic
        auto sum = a1 + a2;
        auto diff = a1 - a2;
        auto scaled = a1 * 2.0f;
        auto divided = a1 / 2.0f;

        init.logger.info("sum: ({}, {})", sum.x, sum.y);
        init.logger.info("diff: ({}, {})", diff.x, diff.y);
        init.logger.info("scaled by 2: ({}, {})", scaled.x, scaled.y);
        init.logger.info("divided by 2: ({}, {})", divided.x, divided.y);

        // Safe division (returns Option)
        auto div_ok = a1.divChecked(2.0f);
        auto div_zero = a1.divChecked(0.0f);

        if (div_ok.isSome()) {
            auto v = div_ok.unwrap();
            init.logger.info("divChecked(2) = ({}, {})", v.x, v.y);
        }
        if (div_zero.isNone()) {
            init.logger.warn("divChecked(0) returned None (division by zero)");
        }

        // Dot product
        auto dot = a1.dot(a2);
        init.logger.info("dot: {}", dot);

        // Normalization
        auto norm = a1.normalized();
        if (norm.isSome()) {
            auto u = norm.unwrap();
            init.logger.info("normalized a1: ({}, {})", u.x, u.y);
        }

        // Zero vector
        auto zero = Vector2f::zero();
        init.logger.info("zero: ({}, {})", zero.x, zero.y);
    }

    // --- Vector3 ---

    {
        init.logger.info("--- Vector3 ---");

        Vector3f b1{.x = 1.0f, .y = 2.0f, .z = 3.0f};
        Vector3f b2 = Vector3f::create(4.0f, 5.0f, 6.0f);

        init.logger.info("b1: ({}, {}, {})", b1.x, b1.y, b1.z);
        init.logger.info("b2: ({}, {}, {})", b2.x, b2.y, b2.z);

        auto sum = b1 + b2;
        auto diff = b1 - b2;
        auto scaled = b1 * 2.0f;
        auto divided = b1 / 2.0f;

        init.logger.info("sum: ({}, {}, {})", sum.x, sum.y, sum.z);
        init.logger.info("diff: ({}, {}, {})", diff.x, diff.y, diff.z);
        init.logger.info("scaled by 2: ({}, {}, {})", scaled.x, scaled.y, scaled.z);
        init.logger.info("divided by 2: ({}, {}, {})", divided.x, divided.y, divided.z);

        // Dot and cross
        auto dot = b1.dot(b2);
        auto cross = b1.cross(b2);

        init.logger.info("dot: {}", dot);
        init.logger.info("cross: ({}, {}, {})", cross.x, cross.y, cross.z);

        // Normalization
        auto norm = b1.normalized();
        if (norm.isSome()) {
            auto u = norm.unwrap();
            init.logger.info("normalized b1: ({}, {}, {})", u.x, u.y, u.z);
        }

        // Zero vector
        auto zero = Vector3f::zero();
        init.logger.info("zero: ({}, {}, {})", zero.x, zero.y, zero.z);
    }

    // --- Quaternion ---

    {
        init.logger.info("--- Quaternion ---");

        // Identity quaternion
        auto q1 = Quaternionf::identity();
        init.logger.info("identity: ({}, {}, {}, {})", q1.x, q1.y, q1.z, q1.w);

        // Create from axis-angle (90° around Z)
        auto axis = Vector3f::create(0.0f, 0.0f, 1.0f);
        auto q2 = Quaternionf::fromAxisAngle(axis, math::pi / 2.0f);
        init.logger.info("fromAxisAngle(90° around Z): ({}, {}, {}, {})",
                         q2.x, q2.y, q2.z, q2.w);

        // Conjugate
        auto conj = q2.conjugate();
        init.logger.info("conjugate: ({}, {}, {}, {})", conj.x, conj.y, conj.z, conj.w);

        // Inverse (returns Option)
        auto inv = q2.inverse();
        if (inv.isSome()) {
            auto q_inv = inv.unwrap();
            init.logger.info("inverse: ({}, {}, {}, {})",
                             q_inv.x, q_inv.y, q_inv.z, q_inv.w);
        }

        // Normalize
        auto norm = q2.normalized();
        if (norm.isSome()) {
            auto qn = norm.unwrap();
            init.logger.info("normalized: ({}, {}, {}, {})",
                             qn.x, qn.y, qn.z, qn.w);
        }

        // Rotate vector
        auto v = Vector3f::create(1.0f, 0.0f, 0.0f);
        auto rotated = q2.rotate(v);
        init.logger.info("rotate (1,0,0) by 90° around Z: ({}, {}, {})",
                         rotated.x, rotated.y, rotated.z);

        // To Euler angles
        auto euler = q2.toEulers();
        init.logger.info("toEulers: (roll={}, pitch={}, yaw={})",
                         euler.x, euler.y, euler.z);

        // Multiply quaternions
        auto q3 = Quaternionf::identity() * q2;
        init.logger.info("identity * q2 = ({}, {}, {}, {})", q3.x, q3.y, q3.z, q3.w);

        // Length and lengthSquared
        init.logger.info("length: {}, lengthSquared: {}", q2.length(), q2.lengthSquared());
    }
}