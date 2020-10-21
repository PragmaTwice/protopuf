#include <gtest/gtest.h>

#include <protopuf/zigzag.h>

using namespace pp;
using namespace std;

GTEST_TEST(static, sint_zigzag) {
    static_assert(pp::is_integral_v<sint_zigzag<1>>);
    static_assert(pp::is_integral_v<sint_zigzag<4>>);
}

GTEST_TEST(sint_zigzag, integer) {
    sint_zigzag<1> z(0), m1(-1), p1(1), m2(-2), p2(2);

    EXPECT_EQ(z.get(), 0);
    EXPECT_EQ(z.get_underlying(), 0);
    EXPECT_EQ(m1.get(), -1);
    EXPECT_EQ(m1.get_underlying(), 1);
    EXPECT_EQ(p1.get(), 1);
    EXPECT_EQ(p1.get_underlying(), 2);
    EXPECT_EQ(m2.get(), -2);
    EXPECT_EQ(m2.get_underlying(), 3);
    EXPECT_EQ(p2.get(), 2);
    EXPECT_EQ(p2.get_underlying(), 4);


    auto uz = sint_zigzag<1>::from_uint(0),
        um1 = sint_zigzag<1>::from_uint(1),
        up1 = sint_zigzag<1>::from_uint(2),
        uz2 = sint_zigzag<1>::from_uint(3),
        up2 = sint_zigzag<1>::from_uint(4);

    EXPECT_EQ(uz.get(), 0);
    EXPECT_EQ(um1.get(), -1);
    EXPECT_EQ(up1.get(), 1);
    EXPECT_EQ(uz2.get(), -2);
    EXPECT_EQ(up2.get(), 2);

    sint_zigzag<4> p10k(10000);
    EXPECT_EQ(p10k.get(), 10000);
    EXPECT_EQ(p10k.get_underlying(), 20000);
}

GTEST_TEST(sint_zigzag, bytes) {
    sint_zigzag<4> p10k(10000);
    array a10k {0x20_b, 0x4e_b, 0_b, 0_b};
    EXPECT_EQ(p10k.dump(), a10k);
}

GTEST_TEST(sint_zigzag, integer_coder) {
    sint_zigzag<4> p10k(10000);
    array a10k {0x20_b, 0x4e_b, 0_b, 0_b};

    array<byte, 4> a10ke {};
    integer_coder<sint_zigzag<4>>::encode(p10k, span(a10ke));
    EXPECT_EQ(a10k, a10ke);

    auto [p10ke, _] = integer_coder<sint_zigzag<4>>::decode(span(a10k));
    EXPECT_EQ(p10k, p10ke);
}

GTEST_TEST(sint_zigzag, varint_coder) {
    sint_zigzag<4> p10k(10000);
    array a10k {0xa0_b, 0x9c_b, 0x01_b, 0_b};

    array<byte, 4> a10ke {};
    varint_coder<sint_zigzag<4>>::encode(p10k, span(a10ke));
    EXPECT_EQ(a10k, a10ke);

    auto [p10ke, _] = varint_coder<sint_zigzag<4>>::decode(span(a10k));
    EXPECT_EQ(p10k, p10ke);
}
