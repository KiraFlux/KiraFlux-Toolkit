#include <kf/StringView.hpp>
#include <runner.hpp>

using kf::Slice;
using kf::StringView;
using kf::TrivialOption;

static void assertStringViewEqual(StringView actual, const char *expected, size_t len) {
    TEST_ASSERT_EQUAL_UINT32(len, actual.length());

    if (len) {
        TEST_ASSERT_EQUAL_MEMORY(expected, actual.data(), len);
    }
}

namespace constructors {

void default_() {
    StringView sv{};

    TEST_ASSERT_NULL(sv.data());
    TEST_ASSERT_EQUAL_UINT32(0, sv.length());
    TEST_ASSERT_TRUE(sv.empty());
}

void cstring() {
    const char *s{"hello"};
    StringView sv{s};

    TEST_ASSERT_EQUAL_PTR(s, sv.data());
    TEST_ASSERT_EQUAL_UINT32(5, sv.length());
}

void ptr_size() {
    const char *d{"world"};
    StringView sv{d, 3};

    TEST_ASSERT_EQUAL_PTR(d, sv.data());
    TEST_ASSERT_EQUAL_UINT32(3, sv.length());
    TEST_ASSERT_EQUAL('w', sv[0]);
    TEST_ASSERT_EQUAL('o', sv[1]);
    TEST_ASSERT_EQUAL('r', sv[2]);
}

void literal() {
    StringView sv{"literal"};

    TEST_ASSERT_EQUAL_UINT32(7, sv.length());
    TEST_ASSERT_EQUAL_STRING("literal", sv.data());
}

void slice() {
    const char *d{"slice"};
    Slice<const char> sl{d, 5};
    StringView sv{d, sl.length()};

    TEST_ASSERT_EQUAL_PTR(d, sv.data());
    TEST_ASSERT_EQUAL_UINT32(5, sv.length());
}

void null_cstring() {
    StringView sv{nullptr};

    TEST_ASSERT_NULL(sv.data());
    TEST_ASSERT_EQUAL_UINT32(0, sv.length());
    TEST_ASSERT_TRUE(sv.empty());
}

}// namespace constructors

namespace test_access {

void iterators() {
    StringView sv{"12345"};
    const char *it{sv.begin()};

    TEST_ASSERT_EQUAL('1', *it);
    it += 1;

    TEST_ASSERT_EQUAL('2', *it);
    it += 1;

    TEST_ASSERT_EQUAL('3', *it);
    it += 1;

    TEST_ASSERT_EQUAL('4', *it);
    it += 1;

    TEST_ASSERT_EQUAL('5', *it);
    it += 1;

    TEST_ASSERT_EQUAL(sv.end(), it);
}

}// namespace test_access

namespace compare {

void equality() {
    StringView a{"hello"};
    StringView b{"hello"};
    StringView c{"world"};

    TEST_ASSERT_TRUE(a == b);
    TEST_ASSERT_FALSE(a != b);
    TEST_ASSERT_TRUE(a != c);
}

}// namespace compare

namespace find {

void character() {
    StringView sv{"hello world"};
    auto p{sv.indexOf('o')};

    TEST_ASSERT_TRUE(p.isSome() and p.unwrap() == 4);

    p = sv.indexOf('o');

    TEST_ASSERT_TRUE(p.isSome() and p.unwrap() == 4);

    StringView sub{sv.sub(5)};
    p = sub.indexOf('o');

    TEST_ASSERT_TRUE(p.isSome() and p.unwrap() == 2);

    p = sv.indexOf('x');

    TEST_ASSERT_TRUE(p.isNone());
}

void substring() {
    StringView sv{"the quick brown fox"};
    auto p{sv.indexOf("quick")};
    TEST_ASSERT_TRUE(p.isSome() and p.unwrap() == 4);

    p = sv.indexOf("brown");
    TEST_ASSERT_TRUE(p.isSome() and p.unwrap() == 0);

    p = sv.indexOf("cat");
    TEST_ASSERT_FALSE(p.isSome());

    p = sv.indexOf("");
    TEST_ASSERT_TRUE(p.isNone());

    p = sv.indexOf('?');
    TEST_ASSERT_TRUE(p.isNone());
}

void rfind() {
    StringView sv{"abracadabra"};
    auto p{sv.lastIndexOf('a')};

    TEST_ASSERT_TRUE(p.isSome() and p.unwrap() == 10);

    StringView sub{sv.sub(0, kf::someTrivial<kf::usize>(5))};
    TEST_ASSERT_EQUAL(5, sub.length());
    
    p = sub.lastIndexOf('a');
    TEST_ASSERT_TRUE(p.isSome() and p.unwrap() == 5);

    p = sv.lastIndexOf('z');
    TEST_ASSERT_TRUE(p.isNone());
}

}// namespace find

namespace starts_ends {

void test() {
    StringView sv{"hello.txt"};

    TEST_ASSERT_TRUE(sv.startsWith("hello"));
    TEST_ASSERT_TRUE(sv.endsWith(".txt"));
    TEST_ASSERT_FALSE(sv.startsWith("world"));
    TEST_ASSERT_FALSE(sv.endsWith(".cpp"));
}

}// namespace starts_ends

namespace sub {

void sub() {
    StringView sv{"hello world"};
    auto s{sv.sub(6, kf::someTrivial<kf::usize>(5))};

    assertStringViewEqual(s, "world", 5);

    s = sv.sub(6, kf::someTrivial<kf::usize>(100));

    assertStringViewEqual(s, "world", 5);

    s = sv.sub(20, kf::someTrivial<kf::usize>(5));

    TEST_ASSERT_TRUE(s.empty());

    s = sv.sub(3, kf::someTrivial<kf::usize>(0));

    TEST_ASSERT_TRUE(s.empty());
}

void from_offset() {
    StringView sv{"hello world"};
    auto s{sv.fromOffset(6)};

    assertStringViewEqual(s, "world", 5);

    s = sv.fromOffset(20);

    TEST_ASSERT_TRUE(s.empty());

    s = sv.fromOffset(0);

    TEST_ASSERT_EQUAL_UINT32(11, s.length());
}

}// namespace sub

namespace crop {

void remove_prefix() {
    StringView sv{"hello world"};
    sv = sv.removePrefix("hello ");

    assertStringViewEqual(sv, "world", 5);

    sv = sv.removePrefix("xyz");

    assertStringViewEqual(sv, "world", 5);
}

void remove_suffix() {
    StringView sv{"hello world"};
    sv = sv.removeSuffix(" world");

    assertStringViewEqual(sv, "hello", 5);

    sv = sv.removeSuffix("xyz");

    assertStringViewEqual(sv, "hello", 5);
}

}// namespace crop

namespace trim {

void start() {
    assertStringViewEqual(StringView{"   hello"}.trimStart(), "hello", 5);
}

void end() {
    assertStringViewEqual(StringView{"hello   "}.trimEnd(), "hello", 5);
}

void both() {
    assertStringViewEqual(StringView{"   hello world   "}.trim(), "hello world", 11);
    assertStringViewEqual(StringView{""}.trim(), "", 0);
    assertStringViewEqual(StringView{"   "}.trim(), "", 0);
    assertStringViewEqual(StringView{"  a  "}.trim(), "a", 1);
}

}// namespace trim

namespace to_slice {

void test() {
    StringView sv{"test"};
    auto sl{sv.slice()};

    TEST_ASSERT_EQUAL_PTR(sv.data(), sl.data());
    TEST_ASSERT_EQUAL_UINT32(sv.length(), sl.length());
}

}// namespace to_slice

void run_tests() {
    RUN_TEST(constructors::default_);
    RUN_TEST(constructors::cstring);
    RUN_TEST(constructors::ptr_size);
    RUN_TEST(constructors::literal);
    RUN_TEST(constructors::slice);
    RUN_TEST(constructors::null_cstring);

    RUN_TEST(test_access::iterators);

    RUN_TEST(compare::equality);

    RUN_TEST(find::character);
    RUN_TEST(find::substring);
    RUN_TEST(find::rfind);

    RUN_TEST(starts_ends::test);

    RUN_TEST(sub::sub);
    RUN_TEST(sub::from_offset);

    RUN_TEST(crop::remove_prefix);
    RUN_TEST(crop::remove_suffix);

    RUN_TEST(trim::start);
    RUN_TEST(trim::end);
    RUN_TEST(trim::both);

    RUN_TEST(to_slice::test);
}