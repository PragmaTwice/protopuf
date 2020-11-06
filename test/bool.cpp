#include <gtest/gtest.h>

#include <protopuf/bool.h>
#include <array>

using namespace pp;
using namespace std;

GTEST_TEST(bool_coder, encode) {
    array<byte, 10> a{};

    EXPECT_EQ(begin_diff(bool_coder::encode(true, a), a), 1);
    EXPECT_EQ(a[0], 1_b);

    EXPECT_EQ(begin_diff(bool_coder::encode(false, a), a), 1);
    EXPECT_EQ(a[0], 0_b);
}

GTEST_TEST(bool_coder, decode) {
    array<byte, 10> a{};

    {
        auto[v, n] = bool_coder::decode(a);
        EXPECT_EQ(begin_diff(n, a), 1);
        EXPECT_EQ(v, false);
    }

    {
        a[0] = 1_b;

        auto[v, n] = bool_coder::decode(a);
        EXPECT_EQ(begin_diff(n, a), 1);
        EXPECT_EQ(v, true);
    }
}
