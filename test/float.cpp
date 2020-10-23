#include <gtest/gtest.h>

#include <protopuf/float.h>

using namespace pp;
using namespace std;

GTEST_TEST(float_coder, encode) {
    array<byte, 4> a{};
    auto n = float_coder<float>::encode(0, a);
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, (array{0_b, 0_b, 0_b, 0_b}));

    n = float_coder<float>::encode(1, a);
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, (array{0_b, 0_b, 0x80_b, 0x3f_b}));

    n = float_coder<float>::encode(-1, a);
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, (array{0_b, 0_b, 0x80_b, 0xbf_b}));

    n = float_coder<float>::encode(1.234, a);
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, (array{0xb6_b, 0xf3_b, 0x9d_b, 0x3f_b}));

    n = float_coder<float>::encode(1.234e5, a);
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, (array{0_b, 0x04_b, 0xf1_b, 0x47_b}));
}

GTEST_TEST(float_coder, decoder) {
    array a{0_b, 0_b, 0_b, 0_b};
    auto [v, n] = float_coder<float>::decode(a);
    EXPECT_EQ(v, 0.);
    EXPECT_EQ(begin_diff(n, a), 4);

    a = {0_b, 0_b, 0x80_b, 0x3f_b};
    tie(v, n) = float_coder<float>::decode(a);
    EXPECT_FLOAT_EQ(v, 1);

    a = {0_b, 0_b, 0x80_b, 0xbf_b};
    tie(v, n) = float_coder<float>::decode(a);
    EXPECT_FLOAT_EQ(v, -1);

    a = {0xb6_b, 0xf3_b, 0x9d_b, 0x3f_b};
    tie(v, n) = float_coder<float>::decode(a);
    EXPECT_FLOAT_EQ(v, 1.234);

    a = {0_b, 0x04_b, 0xf1_b, 0x47_b};
    tie(v, n) = float_coder<float>::decode(a);
    EXPECT_FLOAT_EQ(v, 1.234e5);
}
