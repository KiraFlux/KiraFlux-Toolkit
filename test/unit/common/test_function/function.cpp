#include <kf/Function.hpp>
#include <runner.hpp>

using kf::Function;

static int add(int a, int b) { return a + b; }

struct Mul {
    int factor;
    int operator()(int x) const { return x * factor; }
};

void test_construct_from_lambda_without_capture() {
    Function<int(int, int)> f = [](int a, int b) { return a + b; };
    TEST_ASSERT_EQUAL(42, f(30, 12));
}

void test_construct_from_lambda_with_capture() {
    int multiplier = 5;
    Function<int(int)> f = [multiplier](int x) { return x * multiplier; };
    TEST_ASSERT_EQUAL(25, f(5));
}

void test_construct_from_function_pointer() {
    Function<int(int, int)> f = add;
    TEST_ASSERT_EQUAL(7, f(3, 4));
}

void test_construct_from_functor() {
    Mul mul{10};
    Function<int(int)> f = mul;
    TEST_ASSERT_EQUAL(100, f(10));
}

void test_move_construction() {
    Function<int(int)> f1 = [](int x) { return x + 1; };
    Function<int(int)> f2 = std::move(f1);
    TEST_ASSERT_EQUAL(11, f2(10));
    // moved-from object may be used but not invoked
}

void test_move_assignment() {
    Function<int(int)> f1 = [](int x) { return x + 2; };
    Function<int(int)> f2 = [](int x) { return x * 2; };
    f2 = std::move(f1);
    TEST_ASSERT_EQUAL(12, f2(10));
}

void test_call_with_reference_argument() {
    int value = 42;
    Function<void(int &)> f = [](int &x) { x = 100; };
    f(value);
    TEST_ASSERT_EQUAL(100, value);
}

void test_call_with_void_return() {
    bool called = false;
    Function<void()> f = [&called]() { called = true; };
    f();
    TEST_ASSERT_TRUE(called);
}

void test_capture_max() {
    char buffer[(2 * sizeof(void *))]{};
    Function<void()> f = [buffer]() {};
    f();
}

// large capture should fail (uncomment to test)
// void test_large_capture_should_not_compile() {
//     int big_array[10] = {0};
//     Function<void()> f = [big_array]() {};
// }

void run_tests() {
    test_capture_max();

    RUN_TEST(test_construct_from_lambda_without_capture);
    RUN_TEST(test_construct_from_lambda_with_capture);
    RUN_TEST(test_construct_from_function_pointer);
    RUN_TEST(test_construct_from_functor);
    RUN_TEST(test_move_construction);
    RUN_TEST(test_move_assignment);
    RUN_TEST(test_call_with_reference_argument);
    RUN_TEST(test_call_with_void_return);
}