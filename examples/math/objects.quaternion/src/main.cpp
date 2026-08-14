// KiraFlux-Toolkit Example 'math/objects.quaternion'

#include <kf/main.hpp>
#include <kf/math.hpp>

using namespace kf::math;

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: math/objects.quaternion");

    // --- Identity ---

    // Identity quaternion (no rotation), useful as initial value.
    auto q_id = Quaternionf::identity();
    init.logger.debug("identity: {}", q_id);

    // --- Axis‑angle ---

    // Rotation of 90° around Z axis (common for yaw).
    auto axis = Vector3f::create(0.0f, 0.0f, 1.0f);
    auto q_axis = Quaternionf::fromAxisAngle(axis, pi / 2.0f);
    init.logger.debug("axis-angle (Z, 90°): {}", q_axis);

    // --- Euler angles (ZYX) ---

    // roll, pitch, yaw in radians; intuitive for robotics.
    auto euler_in = Vector3f::create(0.1f, 0.2f, 0.3f);
    auto q_euler = Quaternionf::fromEulers(euler_in);
    init.logger.debug("fromEulers (roll=0.1, pitch=0.2, yaw=0.3): {}", q_euler);

    // --- Conjugate ---

    // Negates imaginary part; used for inverse and rotation (efficiency).
    auto conj = q_axis.conjugate();
    init.logger.debug("conjugate: {}", conj);

    // --- Inverse ---

    // Returns Option to handle zero norm (shouldn't happen for valid quaternions).
    auto inv = q_axis.inverse();
    if (inv.isSome()) {
        init.logger.debug("inverse: {}", inv.unwrap());
    }

    // --- Normalization ---

    // Ensures unit norm (numerical stability); returns Option if zero.
    auto norm = q_axis.normalized();
    if (norm.isSome()) {
        init.logger.debug("normalized: {}", norm.unwrap());
    }

    // --- Rotation of a vector ---

    // Apply rotation to a 3D vector (uses Hamilton product internally).
    auto v = Vector3f::create(1.0f, 0.0f, 0.0f);
    auto rotated = q_axis.rotate(v);
    init.logger.debug("rotate (1,0,0) by 90° around Z: {}", rotated);

    // --- Euler angles extraction ---

    // Convert back to roll-pitch-yaw (useful for debugging orientation).
    auto euler_out = q_axis.toEulers();
    init.logger.debug("toEulers(): (roll={}, pitch={}, yaw={})",
                      euler_out.x, euler_out.y, euler_out.z);

    // --- Multiplication (composition) ---

    // q1 * q2 applies q2 then q1 (order matters).
    auto q_combined = q_id * q_axis;
    init.logger.debug("identity * q_axis = {}", q_combined);

    // In-place multiplication.
    q_combined *= q_axis;
    init.logger.debug("after *= q_axis: {}", q_combined);

    // --- Length and squared length ---

    // length() returns norm; lengthSquared() avoids sqrt (performance).
    init.logger.debug("length: {}", q_axis.length());
    init.logger.debug("lengthSquared: {}", q_axis.lengthSquared());

    // --- Different scalar types ---

    Quaternion<double> qd{.x = 0.0, .y = 0.0, .z = 0.0, .w = 1.0};
    init.logger.debug("double quaternion: {}", qd);
}