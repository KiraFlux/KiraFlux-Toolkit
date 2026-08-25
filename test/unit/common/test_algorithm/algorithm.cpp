#include <kf/math.hpp>
#include <runner.hpp>

using kf::math::clamp;
using kf::math::linearMap;

template<typename T> struct Vals {
    static constexpr T low = T(10);
    static constexpr T high = T(20);
    static constexpr T inside = T(15);
    static constexpr T below = T(5);
    static constexpr T above = T(25);
    static constexpr T out_low = T(100);
    static constexpr T out_high = T(200);
};

template<> struct Vals<float> {
    static constexpr float low = 10.0f;
    static constexpr float high = 20.0f;
    static constexpr float inside = 15.0f;
    static constexpr float below = 5.0f;
    static constexpr float above = 25.0f;
    static constexpr float out_low = 100.0f;
    static constexpr float out_high = 200.0f;
    static constexpr float eps = 1e-5f;
};

template<> struct Vals<double> {
    static constexpr double low = 10.0;
    static constexpr double high = 20.0;
    static constexpr double inside = 15.0;
    static constexpr double below = 5.0;
    static constexpr double above = 25.0;
    static constexpr double out_low = 100.0;
    static constexpr double out_high = 200.0;
    static constexpr double eps = 1e-12;
};

template<typename T> struct AlgorithmTester {
    using V = Vals<T>;

    static void test_clamp() {
        TEST_ASSERT_EQUAL(V::inside, clamp(V::inside, V::low, V::high));
        TEST_ASSERT_EQUAL(V::low, clamp(V::below, V::low, V::high));
        TEST_ASSERT_EQUAL(V::high, clamp(V::above, V::low, V::high));
        TEST_ASSERT_EQUAL(V::low, clamp(V::low, V::low, V::high));
        TEST_ASSERT_EQUAL(V::high, clamp(V::high, V::low, V::high));
    }

    static void test_linearMap() {
        auto low = linearMap(V::low, V::low, V::high, V::out_low, V::out_high);
        auto high = linearMap(V::high, V::low, V::high, V::out_low, V::out_high);
        auto mid = linearMap(V::inside, V::low, V::high, V::out_low, V::out_high);

        if constexpr (std::is_floating_point_v<T>) {
            TEST_ASSERT_FLOAT_WITHIN(V::eps, V::out_low, low);
            TEST_ASSERT_FLOAT_WITHIN(V::eps, V::out_high, high);
            T expected = V::out_low + (V::inside - V::low) * (V::out_high - V::out_low) / (V::high - V::low);
            TEST_ASSERT_FLOAT_WITHIN(V::eps, expected, mid);
        } else {
            TEST_ASSERT_EQUAL(V::out_low, low);
            TEST_ASSERT_EQUAL(V::out_high, high);
            TEST_ASSERT_TRUE(mid >= V::out_low and mid <= V::out_high);
        }
    }
};

#define RUN_ALGORITHM_TESTS(T)                \
    RUN_TEST(AlgorithmTester<T>::test_clamp); \
    RUN_TEST(AlgorithmTester<T>::test_linearMap)

void run_tests() {
    RUN_ALGORITHM_TESTS(int);
    RUN_ALGORITHM_TESTS(unsigned int);
    RUN_ALGORITHM_TESTS(float);
    RUN_ALGORITHM_TESTS(double);
}