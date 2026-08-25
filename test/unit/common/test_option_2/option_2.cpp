#include "structures.hpp"

#include <kf/Option.hpp>
#include <kf/Slice.hpp>

#include <runner.hpp>

using kf::Option;
using kf::Slice;

template<typename T> struct TestOptionTrivial {
    static constexpr T value = T{12345};
    static constexpr T default_value = T{0};

    static void some() {
        auto opt = kf::some(value);
        TEST_ASSERT_TRUE(opt.isSome());
    }

    static void none() {
        Option<T> opt = kf::none;
        TEST_ASSERT_TRUE(opt.isNone());
    }

    static void unwrap() {
        auto opt = kf::some(value);
        TEST_ASSERT_TRUE(opt.unwrap() == value);
    }

    static void unwrap_or_some() {
        auto opt = kf::some(value);
        TEST_ASSERT_TRUE(opt.unwrapOr(default_value) == value);
    }

    static void unwrap_or_none() {
        Option<T> opt = kf::none;
        TEST_ASSERT_TRUE(opt.unwrapOr(default_value) == default_value);
    }

    static void reset() {
        auto opt = kf::some(value);
        opt.reset();
        TEST_ASSERT_TRUE(opt.isNone());
    }

    static void map_some() {
        auto opt = kf::some(value).map([](T v) { return static_cast<float>(v) + 0.5f; });
        TEST_ASSERT_TRUE(opt.isSome());
        TEST_ASSERT_TRUE(opt.unwrap() == static_cast<float>(value) + 0.5f);
    }

    static void void_map_some() {
        bool called = false;
        auto opt = kf::some(value).map([&](T) { called = true; });
        TEST_ASSERT_TRUE(opt.isSome());
        TEST_ASSERT_TRUE(called);
    }

    static void map_none() {
        auto opt = Option<T>{kf::none}.map([](T v) { return v; });
        TEST_ASSERT_TRUE(opt.isNone());
    }

    static void copy() {
        if constexpr (std::is_copy_constructible_v<T>) {
            auto original = kf::some(value);
            auto copy = original;
            TEST_ASSERT_TRUE(copy.isSome());
            TEST_ASSERT_TRUE(copy.unwrap() == original.unwrap());
        }
    }

    static void copy_assignment() {
        if constexpr (std::is_copy_assignable_v<T>) {
            auto original = kf::some(value);
            Option<T> copy = kf::none;
            copy = original;
            TEST_ASSERT_TRUE(copy.isSome());
            TEST_ASSERT_TRUE(copy.unwrap() == original.unwrap());
        }
    }

    static void move() {
        if constexpr (std::is_move_constructible_v<T>) {
            auto original = kf::some(value);
            auto moved = std::move(original);
            TEST_ASSERT_TRUE(moved.isSome());
            TEST_ASSERT_TRUE(moved.unwrap() == value);
            // TEST_ASSERT_TRUE(original.isNone());
        }
    }

    static void move_assignment() {
        if constexpr (std::is_move_assignable_v<T>) {
            auto original = kf::some(value);
            Option<T> moved = kf::none;
            moved = std::move(original);
            TEST_ASSERT_TRUE(moved.isSome());
            TEST_ASSERT_TRUE(moved.unwrap() == value);
            // TEST_ASSERT_TRUE(original.isNone());
        }
    }
};

#define RUN_OPTION2_TRIVIAL_TESTS(__type__)                 \
    RUN_TEST(TestOptionTrivial<__type__>::some);            \
    RUN_TEST(TestOptionTrivial<__type__>::none);            \
    RUN_TEST(TestOptionTrivial<__type__>::unwrap);          \
    RUN_TEST(TestOptionTrivial<__type__>::unwrap_or_some);  \
    RUN_TEST(TestOptionTrivial<__type__>::unwrap_or_none);  \
    RUN_TEST(TestOptionTrivial<__type__>::reset);           \
    RUN_TEST(TestOptionTrivial<__type__>::map_some);        \
    RUN_TEST(TestOptionTrivial<__type__>::void_map_some);   \
    RUN_TEST(TestOptionTrivial<__type__>::map_none);        \
    RUN_TEST(TestOptionTrivial<__type__>::copy);            \
    RUN_TEST(TestOptionTrivial<__type__>::copy_assignment); \
    RUN_TEST(TestOptionTrivial<__type__>::move);            \
    RUN_TEST(TestOptionTrivial<__type__>::move_assignment)

struct TestOptionVoid {
    static void some() {
        auto opt = kf::some();
        TEST_ASSERT_TRUE(opt.isSome());
    }

    static void none() {
        Option<void> opt = kf::none;
        TEST_ASSERT_TRUE(opt.isNone());
    }

    static void reset() {
        auto opt = kf::some();
        opt.reset();
        TEST_ASSERT_TRUE(opt.isNone());
    }
};

template<typename T> struct TestOptionRef {
    inline static T value{};

    static void some() {
        auto opt = kf::someRef(value);
        TEST_ASSERT_TRUE(opt.isSome());
        TEST_ASSERT_TRUE(&opt.unwrap() == &value);
    }

    static void none() {
        Option<T &> opt = kf::none;
        TEST_ASSERT_TRUE(opt.isNone());
    }

    static void copy() {
        auto original = kf::someRef(value);
        auto copy = original;
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(&copy.unwrap() == &value);
    }

    static void copy_assignment() {
        auto original = kf::someRef(value);
        Option<T &> copy = kf::none;
        copy = original;
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(&copy.unwrap() == &value);
    }

    static void move() {
        auto original = kf::someRef(value);
        auto moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(&moved.unwrap() == &value);
    }

    static void move_assignment() {
        auto original = kf::someRef(value);
        Option<T &> moved = kf::none;
        moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(&moved.unwrap() == &value);
    }

    static void reset() {
        auto opt = kf::someRef(value);
        opt.reset();
        TEST_ASSERT_TRUE(opt.isNone());
    }
};

#define RUN_OPTION2_REF_TESTS(__type__, __value__)      \
    TestOptionRef<__type__>::value = __value__;         \
    RUN_TEST(TestOptionRef<__type__>::some);            \
    RUN_TEST(TestOptionRef<__type__>::none);            \
    RUN_TEST(TestOptionRef<__type__>::copy);            \
    RUN_TEST(TestOptionRef<__type__>::copy_assignment); \
    RUN_TEST(TestOptionRef<__type__>::move);            \
    RUN_TEST(TestOptionRef<__type__>::move_assignment); \
    RUN_TEST(TestOptionRef<__type__>::reset)

template<typename T> struct TestOptionSlice {
    static constexpr auto data_a_size = 10u, data_b_size = 20u;
    inline static T data_a[data_a_size]{};
    inline static T data_b[data_b_size]{};
    inline static kf::Slice<T> slice_a{data_a, data_a_size};
    inline static kf::Slice<T> slice_b{data_b, data_b_size};

    static void some() {
        auto opt = kf::some(slice_a);
        TEST_ASSERT_TRUE(opt.isSome());
        auto s = opt.unwrap();
        TEST_ASSERT_EQUAL_PTR(data_a, s.data());
        TEST_ASSERT_EQUAL(data_a_size, s.length());
    }

    static void none() {
        Option<kf::Slice<T>> opt = kf::none;
        TEST_ASSERT_TRUE(opt.isNone());
        auto s = opt.unwrapOr(slice_b);
        TEST_ASSERT_EQUAL_PTR(data_b, s.data());
        TEST_ASSERT_EQUAL(data_b_size, s.length());
    }

    static void copy() {
        auto const original = kf::some(slice_a);
        auto const copy = original;
        TEST_ASSERT_TRUE(copy.isSome());
        auto s = copy.unwrap();
        TEST_ASSERT_EQUAL_PTR(data_a, s.data());
    }

    static void copy_assignment() {
        auto original = kf::some(slice_a);
        Option<kf::Slice<T>> copy = kf::none;
        copy = original;
        TEST_ASSERT_TRUE(copy.isSome());
        auto s = copy.unwrap();
        TEST_ASSERT_EQUAL_PTR(data_a, s.data());
    }

    static void move() {
        auto original = kf::some(slice_a);
        auto moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        auto s = moved.unwrap();
        TEST_ASSERT_EQUAL_PTR(data_a, s.data());
    }

    static void move_assignment() {
        auto original = kf::some(slice_a);
        Option<kf::Slice<T>> moved = kf::none;
        moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        auto s = moved.unwrap();
        TEST_ASSERT_EQUAL_PTR(data_a, s.data());
    }

    static void reset() {
        auto opt = kf::some(slice_a);
        opt.reset();
        TEST_ASSERT_TRUE(opt.isNone());
    }
};

#define RUN_OPTION2_SLICE_TESTS(__type__)                 \
    RUN_TEST(TestOptionSlice<__type__>::some);            \
    RUN_TEST(TestOptionSlice<__type__>::none);            \
    RUN_TEST(TestOptionSlice<__type__>::copy);            \
    RUN_TEST(TestOptionSlice<__type__>::copy_assignment); \
    RUN_TEST(TestOptionSlice<__type__>::move);            \
    RUN_TEST(TestOptionSlice<__type__>::move_assignment); \
    RUN_TEST(TestOptionSlice<__type__>::reset)

void run_tests() {
    using namespace kf::test;

    RUN_OPTION2_TRIVIAL_TESTS(int);
    RUN_OPTION2_TRIVIAL_TESTS(float);
    RUN_OPTION2_TRIVIAL_TESTS(kf::usize);

    RUN_TEST(TestOptionVoid::some);
    RUN_TEST(TestOptionVoid::none);
    RUN_TEST(TestOptionVoid::reset);

    RUN_OPTION2_REF_TESTS(int, 12345);

    RUN_OPTION2_SLICE_TESTS(int);
}