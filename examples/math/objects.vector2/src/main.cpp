// KiraFlux-Toolkit Example 'math/objects.vector2'

#include <kf/main.hpp>
#include <kf/math.hpp>

using namespace kf::math;

void kf::main(kf::Init &init) {
    init.logger.info("KiraFlux-Toolkit Example: math/objects.vector2");

    // --- Construction with different scalar types ---

    // Template works with any arithmetic type (int, float, double).
    // We use aliases for convenience, but you can instantiate directly.
    Vector2f vf{.x = 1.5f, .y = 2.5f};// float
    Vector2i vi{.x = 3, .y = 4};      // int

    // ReprTo prints as "(x, y)" for all types.
    init.logger.debug("float vector: {}", vf);
    init.logger.debug("int vector: {}", vi);

    // Factory method for type deduction (auto casts).
    auto v_auto = Vector2f::create(10, 20);// ints promoted to float
    init.logger.debug("auto-created: {}", v_auto);

    // --- Arithmetic operations ---

    // Operators return new vectors (immutable by default).
    auto sum = vf + v_auto;
    auto diff = vf - v_auto;
    auto scaled = vf * 2.0f;
    auto divided = vf / 2.0f;

    init.logger.debug("sum: {}", sum);
    init.logger.debug("diff: {}", diff);
    init.logger.debug("scaled: {}", scaled);
    init.logger.debug("divided: {}", divided);

    // In-place operators modify the original.
    vf += v_auto;// vf = vf + v_auto
    init.logger.debug("after += : {}", vf);

    vf -= v_auto;
    init.logger.debug("after -= : {}", vf);

    // --- Safe division (avoids division by zero) ---

    // divChecked returns Option to handle zero denominator without exceptions.
    auto ok = vf.divChecked(2.0f);
    auto zero = vf.divChecked(0.0f);

    if (ok.isSome()) {
        init.logger.debug("divChecked(2) = {}", ok.unwrap());
    }
    if (zero.isNone()) {
        init.logger.warn("divChecked(0) -> None (division by zero)");
    }

    // --- Dot product ---

    // Measures projection; useful for angle and similarity.
    auto dot = vf.dot(v_auto);
    init.logger.debug("dot product: {}", dot);

    // --- Normalization (unit vector) ---

    // Returns Option to handle zero-length vectors (prevents NaN).
    auto norm = vf.normalized();
    if (norm.isSome()) {
        init.logger.debug("normalized: {}", norm.unwrap());
    } else {
        init.logger.warn("zero vector cannot be normalized");
    }

    // --- Zero detection ---

    // isZero() checks both components; avoids manual comparison.
    auto zero_vec = Vector2f::zero();
    init.logger.debug("isZero(zero) = {}", zero_vec.isZero());
    init.logger.debug("isZero(vf) = {}", vf.isZero());

    // --- Zero vector factory ---

    // Creates a vector with both components set to 0.
    init.logger.debug("zero vector: {}", Vector2f::zero());

    // --- Length ---

    // length() returns Euclidean norm; for int vectors it's floating.
    init.logger.debug("length of vf: {}", vf.length());
    init.logger.debug("length of vi: {}", vi.length());

    // --- Explicit template instantiation (no alias) ---

    // You can use Vector2<double> directly (no alias needed).
    Vector2<double> v_explicit{.x = 1.0, .y = 2.0};
    init.logger.debug("explicit double vector: {}", v_explicit);
}