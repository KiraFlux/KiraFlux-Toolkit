#include "structures.hpp"
#include <kf/Option2.hpp>
#include <runner.hpp>

using kf::Option2;

template<typename T> struct TestOption2Trivial {
    static constexpr T value = T{12345};
    static constexpr T default_value = T{0};

    static void some() {
        auto opt = kf::some2(value);
        TEST_ASSERT_TRUE(opt.isSome());
    }

    static void none() {
        Option2<T> opt = kf::none;
        TEST_ASSERT_TRUE(opt.isNone());
    }

    static void unwrap() {
        auto opt = kf::some2(value);
        TEST_ASSERT_TRUE(opt.unwrap() == value);
    }

    static void unwrap_or_some() {
        auto opt = kf::some2(value);
        TEST_ASSERT_TRUE(opt.unwrapOr(default_value) == value);
    }

    static void unwrap_or_none() {
        Option2<T> opt = kf::none;
        TEST_ASSERT_TRUE(opt.unwrapOr(default_value) == default_value);
    }

    static void reset() {
        auto opt = kf::some2(value);
        opt.reset();
        TEST_ASSERT_TRUE(opt.isNone());
    }

    static void map_some() {
        auto opt = kf::some2(value).map([](T v) { return static_cast<float>(v) + 0.5f; });
        TEST_ASSERT_TRUE(opt.isSome());
        TEST_ASSERT_TRUE(opt.unwrap() == static_cast<float>(value) + 0.5f);
    }

    static void void_map_some() {
        bool called = false;
        auto opt = kf::some2(value).map([&](T) { called = true; });
        TEST_ASSERT_TRUE(opt.isSome());
        TEST_ASSERT_TRUE(called);
    }

    static void map_none() {
        auto opt = Option2<T>{kf::none}.map([](T v) { return v; });
        TEST_ASSERT_TRUE(opt.isNone());
    }

    static void copy() {
        if constexpr (std::is_copy_constructible_v<T>) {
            auto original = kf::some2(value);
            auto copy = original;
            TEST_ASSERT_TRUE(copy.isSome());
            TEST_ASSERT_TRUE(copy.unwrap() == original.unwrap());
        }
    }

    static void copy_assignment() {
        if constexpr (std::is_copy_assignable_v<T>) {
            auto original = kf::some2(value);
            Option2<T> copy = kf::none;
            copy = original;
            TEST_ASSERT_TRUE(copy.isSome());
            TEST_ASSERT_TRUE(copy.unwrap() == original.unwrap());
        }
    }

    static void move() {
        if constexpr (std::is_move_constructible_v<T>) {
            auto original = kf::some2(value);
            auto moved = std::move(original);
            TEST_ASSERT_TRUE(moved.isSome());
            TEST_ASSERT_TRUE(moved.unwrap() == value);
            // TEST_ASSERT_TRUE(original.isNone());
        }
    }

    static void move_assignment() {
        if constexpr (std::is_move_assignable_v<T>) {
            auto original = kf::some2(value);
            Option2<T> moved = kf::none;
            moved = std::move(original);
            TEST_ASSERT_TRUE(moved.isSome());
            TEST_ASSERT_TRUE(moved.unwrap() == value);
            // TEST_ASSERT_TRUE(original.isNone());
        }
    }
};

#define RUN_OPTION2_TRIVIAL_TESTS(__type__)                  \
    RUN_TEST(TestOption2Trivial<__type__>::some);            \
    RUN_TEST(TestOption2Trivial<__type__>::none);            \
    RUN_TEST(TestOption2Trivial<__type__>::unwrap);          \
    RUN_TEST(TestOption2Trivial<__type__>::unwrap_or_some);  \
    RUN_TEST(TestOption2Trivial<__type__>::unwrap_or_none);  \
    RUN_TEST(TestOption2Trivial<__type__>::reset);           \
    RUN_TEST(TestOption2Trivial<__type__>::map_some);        \
    RUN_TEST(TestOption2Trivial<__type__>::void_map_some);   \
    RUN_TEST(TestOption2Trivial<__type__>::map_none);        \
    RUN_TEST(TestOption2Trivial<__type__>::copy);            \
    RUN_TEST(TestOption2Trivial<__type__>::copy_assignment); \
    RUN_TEST(TestOption2Trivial<__type__>::move);            \
    RUN_TEST(TestOption2Trivial<__type__>::move_assignment)

struct TestOption2Void {
    static void some() {
        auto opt = kf::some2();
        TEST_ASSERT_TRUE(opt.isSome());
    }

    static void none() {
        Option2<void> opt = kf::none;
        TEST_ASSERT_TRUE(opt.isNone());
    }

    static void reset() {
        auto opt = kf::some2();
        opt.reset();
        TEST_ASSERT_TRUE(opt.isNone());
    }
};

template<typename T> struct TestOption2Ref {
    inline static T value{};

    static void some() {
        auto opt = kf::someRef2(value);
        TEST_ASSERT_TRUE(opt.isSome());
        TEST_ASSERT_TRUE(&opt.unwrap() == &value);
    }

    static void none() {
        Option2<T &> opt = kf::none;
        TEST_ASSERT_TRUE(opt.isNone());
    }

    static void copy() {
        auto original = kf::someRef2(value);
        auto copy = original;
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(&copy.unwrap() == &value);
    }

    static void copy_assignment() {
        auto original = kf::someRef2(value);
        Option2<T &> copy = kf::none;
        copy = original;
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(&copy.unwrap() == &value);
    }

    static void move() {
        auto original = kf::someRef2(value);
        auto moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(&moved.unwrap() == &value);
    }

    static void move_assignment() {
        auto original = kf::someRef2(value);
        Option2<T &> moved = kf::none;
        moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(&moved.unwrap() == &value);
    }

    static void reset() {
        auto opt = kf::someRef2(value);
        opt.reset();
        TEST_ASSERT_TRUE(opt.isNone());
    }
};

#define RUN_OPTION2_REF_TESTS(__type__, __value__)       \
    TestOption2Ref<__type__>::value = __value__;         \
    RUN_TEST(TestOption2Ref<__type__>::some);            \
    RUN_TEST(TestOption2Ref<__type__>::none);            \
    RUN_TEST(TestOption2Ref<__type__>::copy);            \
    RUN_TEST(TestOption2Ref<__type__>::copy_assignment); \
    RUN_TEST(TestOption2Ref<__type__>::move);            \
    RUN_TEST(TestOption2Ref<__type__>::move_assignment); \
    RUN_TEST(TestOption2Ref<__type__>::reset)

template<typename T> struct TestOption2Slice {
    static constexpr auto data_a_size = 10u, data_b_size = 20u;
    inline static T data_a[data_a_size]{};
    inline static T data_b[data_b_size]{};
    inline static kf::Slice<T> slice_a{data_a, data_a_size};
    inline static kf::Slice<T> slice_b{data_b, data_b_size};

    static void some() {
        auto opt = kf::some2(slice_a);
        TEST_ASSERT_TRUE(opt.isSome());
        auto s = opt.unwrap();
        TEST_ASSERT_EQUAL_PTR(data_a, s.data());
        TEST_ASSERT_EQUAL(data_a_size, s.length());
    }

    static void none() {
        Option2<kf::Slice<T>> opt = kf::none;
        TEST_ASSERT_TRUE(opt.isNone());
        auto s = opt.unwrapOr(slice_b);
        TEST_ASSERT_EQUAL_PTR(data_b, s.data());
        TEST_ASSERT_EQUAL(data_b_size, s.length());
    }

    static void copy() {
        auto const original = kf::some2(slice_a);
        auto const copy = original;
        TEST_ASSERT_TRUE(copy.isSome());
        auto s = copy.unwrap();
        TEST_ASSERT_EQUAL_PTR(data_a, s.data());
    }

    static void copy_assignment() {
        auto original = kf::some2(slice_a);
        Option2<kf::Slice<T>> copy = kf::none;
        copy = original;
        TEST_ASSERT_TRUE(copy.isSome());
        auto s = copy.unwrap();
        TEST_ASSERT_EQUAL_PTR(data_a, s.data());
    }

    static void move() {
        auto original = kf::some2(slice_a);
        auto moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        auto s = moved.unwrap();
        TEST_ASSERT_EQUAL_PTR(data_a, s.data());
    }

    static void move_assignment() {
        auto original = kf::some2(slice_a);
        Option2<kf::Slice<T>> moved = kf::none;
        moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        auto s = moved.unwrap();
        TEST_ASSERT_EQUAL_PTR(data_a, s.data());
    }

    static void reset() {
        auto opt = kf::some2(slice_a);
        opt.reset();
        TEST_ASSERT_TRUE(opt.isNone());
    }
};

#define RUN_OPTION2_SLICE_TESTS(__type__)                  \
    RUN_TEST(TestOption2Slice<__type__>::some);            \
    RUN_TEST(TestOption2Slice<__type__>::none);            \
    RUN_TEST(TestOption2Slice<__type__>::copy);            \
    RUN_TEST(TestOption2Slice<__type__>::copy_assignment); \
    RUN_TEST(TestOption2Slice<__type__>::move);            \
    RUN_TEST(TestOption2Slice<__type__>::move_assignment); \
    RUN_TEST(TestOption2Slice<__type__>::reset)

void run_tests() {
    using namespace kf::test;

    RUN_OPTION2_TRIVIAL_TESTS(int);
    RUN_OPTION2_TRIVIAL_TESTS(float);
    RUN_OPTION2_TRIVIAL_TESTS(kf::usize);

    RUN_TEST(TestOption2Void::some);
    RUN_TEST(TestOption2Void::none);
    RUN_TEST(TestOption2Void::reset);

    RUN_OPTION2_REF_TESTS(int, 12345);

    RUN_OPTION2_SLICE_TESTS(int);
}