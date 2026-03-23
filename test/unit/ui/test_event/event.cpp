#include <kf/ui/Event.hpp>
#include <unity.h>

using namespace kf::ui;

template<kf::u8 V> struct EventTester {
    using Ev = Event<V>;
    using Value = typename Ev::Value;

    static void test_basic() {
        Value test_val = (V == 1) ? 0 : 1;
        Ev ev(Ev::Kind::WidgetClick, test_val);
        TEST_ASSERT_EQUAL(Ev::Kind::WidgetClick, ev.kind());
        TEST_ASSERT_EQUAL(test_val, ev.value());
    }

    static void test_boundaries() {
        if constexpr (V > 1) {
            TEST_ASSERT(Ev::value_max > 0);
        }
        TEST_ASSERT(Ev::value_min < 0);

        Ev ev_min(Ev::Kind::PageCursorMove, Ev::value_min);
        Ev ev_max(Ev::Kind::PageCursorMove, Ev::value_max);
        TEST_ASSERT_EQUAL(Ev::value_min, ev_min.value());
        TEST_ASSERT_EQUAL(Ev::value_max, ev_max.value());
    }

    static void test_factories() {
        auto u = Ev::update();
        TEST_ASSERT_EQUAL(Ev::Kind::Update, u.kind());
        TEST_ASSERT_EQUAL(0, u.value());

        // Для V=1 используем 0, иначе 1
        Value move_val = (V == 1) ? 0 : 1;
        auto move = Ev::pageCursorMove(move_val);
        TEST_ASSERT_EQUAL(Ev::Kind::PageCursorMove, move.kind());
        TEST_ASSERT_EQUAL(move_val, move.value());

        auto click = Ev::widgetClick();
        TEST_ASSERT_EQUAL(Ev::Kind::WidgetClick, click.kind());
        TEST_ASSERT_EQUAL(0, click.value());

        Value widget_val = -1;
        auto val = Ev::widgetValue(widget_val);
        TEST_ASSERT_EQUAL(Ev::Kind::WidgetValueChange, val.kind());
        TEST_ASSERT_EQUAL(widget_val, val.value());
    }

    static void test_independence() {
        Ev ev(Ev::Kind::WidgetValueChange, Ev::value_max);
        TEST_ASSERT_EQUAL(Ev::Kind::WidgetValueChange, ev.kind());
        TEST_ASSERT_EQUAL(Ev::value_max, ev.value());
    }

    static void test_zero() {
        Ev zero1(Ev::Kind::Update, 0);
        Ev zero2(Ev::Kind::WidgetClick, 0);
        TEST_ASSERT_EQUAL(0, zero1.value());
        TEST_ASSERT_EQUAL(0, zero2.value());
        TEST_ASSERT_EQUAL(Ev::Kind::Update, zero1.kind());
        TEST_ASSERT_EQUAL(Ev::Kind::WidgetClick, zero2.kind());
    }
};

#define RUN_EVENT_TESTS(V)                       \
    RUN_TEST(EventTester<V>::test_basic);        \
    RUN_TEST(EventTester<V>::test_boundaries);   \
    RUN_TEST(EventTester<V>::test_factories);    \
    RUN_TEST(EventTester<V>::test_independence); \
    RUN_TEST(EventTester<V>::test_zero)

int main() {
    UNITY_BEGIN();
    RUN_EVENT_TESTS(1);
    RUN_EVENT_TESTS(2);
    RUN_EVENT_TESTS(3);
    RUN_EVENT_TESTS(4);
    RUN_EVENT_TESTS(5);
    RUN_EVENT_TESTS(6);
    RUN_EVENT_TESTS(7);
    RUN_EVENT_TESTS(8);
    RUN_EVENT_TESTS(15);
    RUN_EVENT_TESTS(16);
    RUN_EVENT_TESTS(31);
    RUN_EVENT_TESTS(32);
    RUN_EVENT_TESTS(62);
    return UNITY_END();
}