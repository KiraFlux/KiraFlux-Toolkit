#include <kf/Function.hpp>
#include <kf/Option.hpp>

#include <unity.h>

using kf::Function;
using kf::Option;

static int add(int a, int b) { return a + b; }

template<typename T> struct TestOptionalFunction {
    using FunctionType = Function<T(T)>;

    static_assert(sizeof(Option<FunctionType>) == sizeof(FunctionType));

    static inline int counter = 0;
    static int normal(int x) { return x; }
    static int plus_one(int x) { return x + 1; }

    static void some() {
        FunctionType func = plus_one;
        auto option = kf::some(std::move(func));
        TEST_ASSERT_TRUE(option.isSome());
        TEST_ASSERT_FALSE(option.isNone());
        TEST_ASSERT_EQUAL(42, option.unwrap()(41));// plus_one(41)
    }

    static void none() {
        Option<FunctionType> option = kf::none;
        TEST_ASSERT_TRUE(option.isNone());
        TEST_ASSERT_FALSE(option.isSome());
    }

    static void move_construction() {
        FunctionType func = normal;
        auto option_1 = kf::some(std::move(func));
        auto option_2 = std::move(option_1);
        TEST_ASSERT_TRUE(option_2.isSome());
        TEST_ASSERT_EQUAL(100, option_2.unwrap()(100));
        TEST_ASSERT_TRUE(option_1.isNone());// moved-from becomes None
    }

    static void move_assignment() {
        FunctionType f1 = plus_one;
        FunctionType f2 = normal;
        auto option_1 = kf::some(std::move(f1));
        auto option_2 = kf::some(std::move(f2));
        option_2 = std::move(option_1);
        TEST_ASSERT_TRUE(option_2.isSome());
        TEST_ASSERT_EQUAL(43, option_2.unwrap()(42));
        TEST_ASSERT_TRUE(option_1.isNone());
    }

    static void reset() {
        FunctionType func = normal;
        auto option = kf::some(std::move(func));
        option.reset();
        TEST_ASSERT_TRUE(option.isNone());
    }

    static void value_invocation() {
        FunctionType func = [](int x) { return x * 2; };
        auto option = kf::some(std::move(func));
        TEST_ASSERT_EQUAL(20, option.unwrap()(10));
    }
};

#define RUN_OPTIONAL_FUNCTION_TESTS(__type__)                    \
    RUN_TEST(TestOptionalFunction<__type__>::some);              \
    RUN_TEST(TestOptionalFunction<__type__>::none);              \
    RUN_TEST(TestOptionalFunction<__type__>::move_construction); \
    RUN_TEST(TestOptionalFunction<__type__>::move_assignment);   \
    RUN_TEST(TestOptionalFunction<__type__>::reset);             \
    RUN_TEST(TestOptionalFunction<__type__>::value_invocation)

int main() {
    UNITY_BEGIN();

    RUN_OPTIONAL_FUNCTION_TESTS(int);

    return UNITY_END();
}