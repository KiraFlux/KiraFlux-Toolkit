#include <kf/String.hpp>
#include <runner.hpp>

void test() {

    char buffer[100];

    kf::String my_string{{buffer}};

    my_string.appendFormat("Hello {}", "World");
}

void run_tests() {
    RUN_TEST(test);
}
