#include <kf/Option.hpp>
#include <runner.hpp>

using kf::Option;

template<typename T> struct TestOptionalReference {
    static_assert(sizeof(Option<T &>) == sizeof(T *));

    inline static T value{};
    inline static T default_value{};

    static void some() {
        auto option = kf::someRef(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_FALSE(option.isNone());
        TEST_ASSERT_TRUE(&value == &option.unwrap());
        T &def = default_value;
        TEST_ASSERT_TRUE(&value == &option.unwrapOr(def));
    }

    static void none() {
        Option<T &> option = kf::none;
        TEST_ASSERT_TRUE(option.isNone());
        TEST_ASSERT_FALSE(option.isSome());
        T &def = default_value;
        TEST_ASSERT_TRUE(&def == &option.unwrapOr(def));
    }

    static void copy() {
        auto original = kf::someRef(value);
        auto copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(&value == &original.unwrap());
        TEST_ASSERT_TRUE(&value == &copy.unwrap());
        TEST_ASSERT_TRUE(&original.unwrap() == &copy.unwrap());
    }

    static void copy_assignment() {
        auto original = kf::someRef(value);
        Option<T &> copy = kf::none;
        copy = original;
        TEST_ASSERT_TRUE(original.isSome());
        TEST_ASSERT_TRUE(copy.isSome());
        TEST_ASSERT_TRUE(&value == &original.unwrap());
        TEST_ASSERT_TRUE(&value == &copy.unwrap());
    }

    static void move() {
        auto original = kf::someRef(value);
        auto moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(&value == &moved.unwrap());
    }

    static void move_assignment() {
        auto original = kf::someRef(value);
        Option<T &> moved = kf::none;
        moved = std::move(original);
        TEST_ASSERT_TRUE(moved.isSome());
        TEST_ASSERT_TRUE(&value == &moved.unwrap());
    }

    static void const_instance() {
        const auto option = kf::someRef(value);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(&value == &option.unwrap());
    }

    static void value_get() {
        auto option = kf::someRef(value);
        TEST_ASSERT_TRUE(&value == &option.unwrap());
    }

    static void reassign() {
        T other = default_value;
        auto option = kf::someRef(value);
        option = kf::someRef(other);
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_TRUE(&other == &option.unwrap());

        option = kf::none;
        TEST_ASSERT_TRUE(option.isNone());
    }

    static void reset() {
        auto option = kf::someRef(value);
        option.reset();
        TEST_ASSERT_TRUE(option.isNone());
    }
};

#define RUN_OPTIONAL_REFERENCE_TESTS(__type__, __value__)       \
    TestOptionalReference<__type__>::value = __value__;         \
    RUN_TEST(TestOptionalReference<__type__>::some);            \
    RUN_TEST(TestOptionalReference<__type__>::none);            \
    RUN_TEST(TestOptionalReference<__type__>::copy);            \
    RUN_TEST(TestOptionalReference<__type__>::copy_assignment); \
    RUN_TEST(TestOptionalReference<__type__>::move);            \
    RUN_TEST(TestOptionalReference<__type__>::move_assignment); \
    RUN_TEST(TestOptionalReference<__type__>::const_instance);  \
    RUN_TEST(TestOptionalReference<__type__>::value_get);       \
    RUN_TEST(TestOptionalReference<__type__>::reassign);        \
    RUN_TEST(TestOptionalReference<__type__>::reset)

void run_tests() {
    RUN_OPTIONAL_REFERENCE_TESTS(int, 12345);
}