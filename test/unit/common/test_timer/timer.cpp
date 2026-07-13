// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#include <kf/Timer.hpp>
#include <unity.h>

using kf::math::Milliseconds;
using kf::Timer;

constexpr Milliseconds
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

void test_config_ctor() {
    Timer::Config config{.value = period_100};
    Timer timer{config};
    TEST_ASSERT_EQUAL(period_100, timer.config().value);
}

void test_config_reference() {
    Timer::Config config{.value = period_50};
    Timer timer{config};
    TEST_ASSERT_EQUAL(period_50, timer.config().value);

    config.value = period_200;
    TEST_ASSERT_EQUAL(period_200, timer.config().value);
}

void test_restart() {
    Timer::Config config{.value = period_100};
    Timer timer{config};

    timer.start(t_0);
    TEST_ASSERT_FALSE(timer.expired(t_50));
    TEST_ASSERT_EQUAL(t_50, timer.elapsed(t_50));
    TEST_ASSERT_EQUAL(period_100 - t_50, timer.remaining(t_50));

    timer.start(t_50);
    TEST_ASSERT_EQUAL(t_0, timer.elapsed(t_50));
    TEST_ASSERT_FALSE(timer.expired(t_100));
    TEST_ASSERT_EQUAL(t_50, timer.elapsed(t_100));
}

void test_expired() {
    Timer::Config config{.value = period_100};
    Timer timer{config};
    timer.start(t_0);
    TEST_ASSERT_FALSE(timer.expired(t_50));
    TEST_ASSERT_TRUE(timer.expired(t_100));
    TEST_ASSERT_TRUE(timer.expired(t_150));
}

void test_elapsed() {
    Timer::Config config{.value = period_100};
    Timer timer{config};
    timer.start(t_50);
    TEST_ASSERT_EQUAL(t_0, timer.elapsed(t_50));
    TEST_ASSERT_EQUAL(t_50, timer.elapsed(t_100));
    TEST_ASSERT_EQUAL(t_100, timer.elapsed(t_150));
}

void test_remaining() {
    Timer::Config config{.value = period_100};
    Timer timer{config};
    timer.start(t_0);
    TEST_ASSERT_EQUAL(period_100, timer.remaining(t_0));
    TEST_ASSERT_EQUAL(period_100 - t_50, timer.remaining(t_50));
    TEST_ASSERT_EQUAL(period_0, timer.remaining(t_100));
    TEST_ASSERT_EQUAL(period_0, timer.remaining(t_150));
}

void test_zero_period() {
    Timer::Config config{.value = period_0};
    Timer timer{config};

    timer.start(t_50);
    TEST_ASSERT_TRUE(timer.expired(t_50));
    TEST_ASSERT_EQUAL(period_0, timer.elapsed(t_50));
    TEST_ASSERT_EQUAL(period_0, timer.remaining(t_50));

    timer.start(t_100);
    TEST_ASSERT_TRUE(timer.expired(t_100));
}

void test_unsigned_wrap() {
    Timer::Config config{.value = period_100};
    Timer timer{config};
    timer.start(near_max);

    TEST_ASSERT_FALSE(timer.expired(near_max));
    TEST_ASSERT_EQUAL(period_0, timer.elapsed(near_max));
    TEST_ASSERT_EQUAL(period_100, timer.remaining(near_max));

    const auto later_a = near_max + small_inc;
    TEST_ASSERT_EQUAL(small_inc, timer.elapsed(later_a));
    TEST_ASSERT_FALSE(timer.expired(later_a));
    TEST_ASSERT_EQUAL(period_100 - small_inc, timer.remaining(later_a));

    const auto later_b = near_max + large_inc;
    TEST_ASSERT_EQUAL(large_inc, timer.elapsed(later_b));
    TEST_ASSERT_TRUE(timer.expired(later_b));
    TEST_ASSERT_EQUAL(period_0, timer.remaining(later_b));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_config_ctor);
    RUN_TEST(test_config_reference);
    RUN_TEST(test_restart);
    RUN_TEST(test_expired);
    RUN_TEST(test_elapsed);
    RUN_TEST(test_remaining);
    RUN_TEST(test_zero_period);
    RUN_TEST(test_unsigned_wrap);
    return UNITY_END();
}