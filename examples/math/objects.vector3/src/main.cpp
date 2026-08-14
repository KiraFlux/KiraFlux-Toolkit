// KiraFlux-Toolkit Example 'math/objects.vector3'

#include <kf/main.hpp>
#include <kf/math.hpp>

using namespace kf::math;

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: math/objects.vector3");

    // --- Construction ---

    Vector3f v1{.x = 1.0f, .y = 2.0f, .z = 3.0f};
    Vector3f v2 = Vector3f::create(4.0f, 5.0f, 6.0f);

    // ReprTo prints "(x, y, z)".
    init.logger.debug("v1: {}", v1);
    init.logger.debug("v2: {}", v2);

    // --- Arithmetic (same as Vector2) ---

    auto sum = v1 + v2;
    auto diff = v1 - v2;
    auto scaled = v1 * 2.0f;
    auto divided = v1 / 2.0f;

    init.logger.debug("sum: {}", sum);
    init.logger.debug("diff: {}", diff);
    init.logger.debug("scaled: {}", scaled);
    init.logger.debug("divided: {}", divided);

    // In-place operators also work.
    v1 += v2;
    init.logger.debug("after += : {}", v1);
    v1 -= v2;
    init.logger.debug("after -= : {}", v1);

    // --- Dot and cross products ---

    // Dot: scalar; cross: perpendicular vector (3D only).
    init.logger.debug("dot: {}", v1.dot(v2));
    auto cross = v1.cross(v2);
    init.logger.debug("cross: {}", cross);

    // --- Safe division, normalization, zero check ---

    auto norm = v1.normalized();
    if (norm.isSome()) {
        init.logger.debug("normalized: {}", norm.unwrap());
    }

    auto zero_vec = Vector3f::zero();
    init.logger.debug("isZero(zero) = {}", zero_vec.isZero());

    // --- All methods from Vector2 are also available ---

    // length, divChecked, etc. (not repeated for brevity).
    init.logger.debug("length: {}", v1.length());

    // --- Different scalar types ---

    Vector3<int> vi{.x = 1, .y = 2, .z = 3};
    Vector3<double> vd{.x = 1.5, .y = 2.5, .z = 3.5};
    init.logger.debug("int vector: {}", vi);
    init.logger.debug("double vector: {}", vd);
}