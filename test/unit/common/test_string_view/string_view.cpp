#include <kf/memory/StringView.hpp>
#include <unity.h>

using kf::memory::StringView;
using kf::memory::Slice;

// StringView equal helper
static void assertStringViewEqual(StringView actual, const char *expected, size_t len) {
    TEST_ASSERT_EQUAL_UINT32(len, actual.size());
    if (len) {
        TEST_ASSERT_EQUAL_MEMORY(expected, actual.data(), len);
    }
}

namespace constructors {
void default_() {
    StringView sv;
    TEST_ASSERT_NULL(sv.data());
    TEST_ASSERT_EQUAL_UINT32(0, sv.size());
    TEST_ASSERT_TRUE(sv.empty());
}

void cstring() {
    const char *s = "hello";
    StringView sv(s);
    TEST_ASSERT_EQUAL_PTR(s, sv.data());
    TEST_ASSERT_EQUAL_UINT32(5, sv.size());
}

void ptr_size() {
    const char *d = "world";
    StringView sv(d, 3);
    TEST_ASSERT_EQUAL_PTR(d, sv.data());
    TEST_ASSERT_EQUAL_UINT32(3, sv.size());
    TEST_ASSERT_EQUAL('w', sv[0]);
    TEST_ASSERT_EQUAL('o', sv[1]);
    TEST_ASSERT_EQUAL('r', sv[2]);
}

void literal() {
    StringView sv("literal");
    TEST_ASSERT_EQUAL_UINT32(7, sv.size());
    TEST_ASSERT_EQUAL_STRING("literal", sv.data());
}

void slice() {
    const char *d = "slice";
    Slice<const char> sl(d, 5);
    StringView sv(sl);
    TEST_ASSERT_EQUAL_PTR(d, sv.data());
    TEST_ASSERT_EQUAL_UINT32(5, sv.size());
}

void null_cstring() {
    StringView sv(nullptr);
    TEST_ASSERT_NULL(sv.data());
    TEST_ASSERT_EQUAL_UINT32(0, sv.size());
    TEST_ASSERT_TRUE(sv.empty());
}
}// namespace constructors

namespace access {
void index_and_front_back() {
    StringView sv("abcde");
    TEST_ASSERT_EQUAL('a', sv[0]);
    TEST_ASSERT_EQUAL('e', sv[4]);
    TEST_ASSERT_EQUAL('a', sv.front());
    TEST_ASSERT_EQUAL('e', sv.back());
}

void iterators() {
    StringView sv("12345");
    const char *it = sv.begin();
    TEST_ASSERT_EQUAL('1', *it++);
    TEST_ASSERT_EQUAL('2', *it++);
    TEST_ASSERT_EQUAL('3', *it++);
    TEST_ASSERT_EQUAL('4', *it++);
    TEST_ASSERT_EQUAL('5', *it++);
    TEST_ASSERT_EQUAL(sv.end(), it);
}
}// namespace access

namespace compare {
void equality() {
    StringView a("hello"), b("hello"), c("world");
    TEST_ASSERT_TRUE(a == b);
    TEST_ASSERT_FALSE(a != b);
    TEST_ASSERT_TRUE(a != c);
}

void ordering() {
    StringView a("apple"), b("banana");
    TEST_ASSERT_TRUE(a < b);
    TEST_ASSERT_TRUE(a <= b);
    TEST_ASSERT_FALSE(a > b);
    TEST_ASSERT_FALSE(a >= b);
}

void length_different() {
    StringView a("hello"), b("hello world"), c("he");
    TEST_ASSERT_TRUE(a < b);
    TEST_ASSERT_TRUE(a > c);
}
}// namespace compare

namespace find {
void character() {
    StringView sv("hello world");
    auto p = sv.find('o');
    TEST_ASSERT_TRUE(p.hasValue() && p.value() == 4);
    p = sv.find('o', 5);
    TEST_ASSERT_TRUE(p.hasValue() && p.value() == 7);
    p = sv.find('x');
    TEST_ASSERT_FALSE(p.hasValue());
}

void substring() {
    StringView sv("the quick brown fox");
    auto p = sv.find("quick");
    TEST_ASSERT_TRUE(p.hasValue() && p.value() == 4);
    p = sv.find("brown", 10);
    TEST_ASSERT_TRUE(p.hasValue() && p.value() == 10);
    p = sv.find("cat");
    TEST_ASSERT_FALSE(p.hasValue());
    p = sv.find("");
    TEST_ASSERT_TRUE(p.hasValue() && p.value() == 0);
}

void rfind() {
    StringView sv("abracadabra");
    auto p = sv.rfind('a');
    TEST_ASSERT_TRUE(p.hasValue() && p.value() == 10);
    p = sv.rfind('a', 5);
    TEST_ASSERT_TRUE(p.hasValue() && p.value() == 5);
    p = sv.rfind('z');
    TEST_ASSERT_FALSE(p.hasValue());
}
}// namespace find

namespace starts_ends {
void test() {
    StringView sv("hello.txt");
    TEST_ASSERT_TRUE(sv.startsWith("hello"));
    TEST_ASSERT_TRUE(sv.endsWith(".txt"));
    TEST_ASSERT_FALSE(sv.startsWith("world"));
    TEST_ASSERT_FALSE(sv.endsWith(".cpp"));
}
}// namespace starts_ends

namespace sub {
void sub() {
    StringView sv("hello world");
    auto s = sv.sub(6, 5);
    assertStringViewEqual(s, "world", 5);
    s = sv.sub(6, 100);
    assertStringViewEqual(s, "world", 5);
    s = sv.sub(20, 5);
    TEST_ASSERT_TRUE(s.empty());
    s = sv.sub(3, 0);
    TEST_ASSERT_TRUE(s.empty());
}

void sub_from() {
    StringView sv("hello world");
    auto s = sv.subFrom(6);
    assertStringViewEqual(s, "world", 5);
    s = sv.subFrom(20);
    TEST_ASSERT_TRUE(s.empty());
    s = sv.subFrom(0);
    TEST_ASSERT_EQUAL_UINT32(11, s.size());
}
}// namespace sub

namespace modify {
void remove_prefix() {
    StringView sv("hello world");
    sv.removePrefix(6);
    assertStringViewEqual(sv, "world", 5);
    sv.removePrefix(10);
    TEST_ASSERT_TRUE(sv.empty());
}

void remove_suffix() {
    StringView sv("hello world");
    sv.removeSuffix(6);
    assertStringViewEqual(sv, "hello", 5);
}
}// namespace modify

namespace trim {
void start() {
    assertStringViewEqual(StringView("   hello").trimStart(), "hello", 5);
}

void end() {
    assertStringViewEqual(StringView("hello   ").trimEnd(), "hello", 5);
}

void both() {
    assertStringViewEqual(StringView("   hello world   ").trim(), "hello world", 11);
    assertStringViewEqual(StringView("").trim(), "", 0);
    assertStringViewEqual(StringView("   ").trim(), "", 0);
    assertStringViewEqual(StringView("  a  ").trim(), "a", 1);
}
}// namespace trim

namespace to_slice {
void test() {
    StringView sv("test");
    auto sl = sv.slice();
    TEST_ASSERT_EQUAL_PTR(sv.data(), sl.data());
    TEST_ASSERT_EQUAL_UINT32(sv.size(), sl.size());
}
}// namespace to_slice

int main() {
    UNITY_BEGIN();

    RUN_TEST(constructors::default_);
    RUN_TEST(constructors::cstring);
    RUN_TEST(constructors::ptr_size);
    RUN_TEST(constructors::literal);
    RUN_TEST(constructors::slice);
    RUN_TEST(constructors::null_cstring);

    RUN_TEST(access::index_and_front_back);
    RUN_TEST(access::iterators);

    RUN_TEST(compare::equality);
    RUN_TEST(compare::ordering);
    RUN_TEST(compare::length_different);

    RUN_TEST(find::character);
    RUN_TEST(find::substring);
    RUN_TEST(find::rfind);

    RUN_TEST(starts_ends::test);

    RUN_TEST(sub::sub);
    RUN_TEST(sub::sub_from);

    RUN_TEST(modify::remove_prefix);
    RUN_TEST(modify::remove_suffix);

    RUN_TEST(trim::start);
    RUN_TEST(trim::end);
    RUN_TEST(trim::both);

    RUN_TEST(to_slice::test);

    return UNITY_END();
}