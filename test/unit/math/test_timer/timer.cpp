#include <kf/math/Timer.hpp>

#include <unity.h>

using kf::math::Timer;

constexpr kf::math::Milliseconds
    t_0 = 0,
    t_50 = 50,
    t_100 = 100,
    t_150 = 150,

    period_0 = 0,
    period_50 = 50,
    period_100 = 100,
    period_200 = 200,

    near_max = 0xFFFFFFF0U,
    small_inc = 30,
    large_inc = 150;

void test_default_ctor() {
    Timer t{};
    TEST_ASSERT_EQUAL(period_0, t.period());
    TEST_ASSERT_FALSE(t.expired(t_0));
    TEST_ASSERT_EQUAL(period_0, t.elapsed(t_0));
    TEST_ASSERT_EQUAL(period_0, t.remaining(t_0));
}

void test_period_ctor() {
    Timer t{period_100};
    TEST_ASSERT_EQUAL(period_100, t.period());
}

void test_frequency_ctor() {
    Timer t{kf::math::Hertz(10)};
    TEST_ASSERT_EQUAL(period_100, t.period());
}

void test_period_property() {
    Timer t{period_50};
    TEST_ASSERT_EQUAL(period_50, t.period());
    t.period(period_200);
    TEST_ASSERT_EQUAL(period_200, t.period());
}

void test_start_stop() {
    Timer t{period_100};
    t.start(t_0);

    TEST_ASSERT_FALSE(t.expired(t_50));
    TEST_ASSERT_EQUAL(t_50, t.elapsed(t_50));
    TEST_ASSERT_EQUAL(period_100 - t_50, t.remaining(t_50));

    t.stop();

    TEST_ASSERT_FALSE(t.expired(t_100));
    TEST_ASSERT_EQUAL(t_100, t.elapsed(t_100));
    TEST_ASSERT_EQUAL(period_0, t.remaining(t_100));
}

void test_expired() {
    Timer t{period_100};
    t.start(t_0);
    TEST_ASSERT_FALSE(t.expired(t_50));
    TEST_ASSERT_TRUE(t.expired(t_100));
    TEST_ASSERT_TRUE(t.expired(t_150));
}

void test_elapsed() {
    Timer t{period_100};
    t.start(t_50);
    TEST_ASSERT_EQUAL(t_0, t.elapsed(t_50));
    TEST_ASSERT_EQUAL(t_50, t.elapsed(t_100));
    TEST_ASSERT_EQUAL(t_100, t.elapsed(t_150));
}

void test_remaining() {
    Timer t{period_100};
    t.start(t_0);
    TEST_ASSERT_EQUAL(period_100, t.remaining(t_0));
    TEST_ASSERT_EQUAL(period_100 - t_50, t.remaining(t_50));
    TEST_ASSERT_EQUAL(period_0, t.remaining(t_100));
    TEST_ASSERT_EQUAL(period_0, t.remaining(t_150));
}

void test_after_stop() {
    Timer t{period_100};
    t.start(t_0);
    t.stop();

    TEST_ASSERT_FALSE(t.expired(t_50));
    TEST_ASSERT_EQUAL(t_50, t.elapsed(t_50));
    TEST_ASSERT_EQUAL(period_0, t.remaining(t_50));
}

void test_zero_period() {
    Timer t{period_0};
    t.start(t_50);
    TEST_ASSERT_TRUE(t.expired(t_50));
    TEST_ASSERT_EQUAL(period_0, t.elapsed(t_50));
    TEST_ASSERT_EQUAL(period_0, t.remaining(t_50));

    t.start(t_100);
    TEST_ASSERT_TRUE(t.expired(t_100));
}

void test_unsigned_wrap() {
    Timer t{period_100};
    t.start(near_max);

    TEST_ASSERT_FALSE(t.expired(near_max));
    TEST_ASSERT_EQUAL(period_0, t.elapsed(near_max));
    TEST_ASSERT_EQUAL(period_100, t.remaining(near_max));

    const auto later_a = near_max + small_inc;
    TEST_ASSERT_EQUAL(small_inc, t.elapsed(later_a));
    TEST_ASSERT_FALSE(t.expired(later_a));
    TEST_ASSERT_EQUAL(period_100 - small_inc, t.remaining(later_a));

    const auto later_b = near_max + large_inc;
    TEST_ASSERT_EQUAL(large_inc, t.elapsed(later_b));
    TEST_ASSERT_TRUE(t.expired(later_b));
    TEST_ASSERT_EQUAL(period_0, t.remaining(later_b));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_default_ctor);
    RUN_TEST(test_period_ctor);
    RUN_TEST(test_frequency_ctor);
    RUN_TEST(test_period_property);
    RUN_TEST(test_start_stop);
    RUN_TEST(test_expired);
    RUN_TEST(test_elapsed);
    RUN_TEST(test_remaining);
    RUN_TEST(test_after_stop);
    RUN_TEST(test_zero_period);
    RUN_TEST(test_unsigned_wrap);
    return UNITY_END();
}
