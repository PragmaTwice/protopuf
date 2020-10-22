#include <gtest/gtest.h>

#include <protopuf/varint.h>

using namespace pp;
using namespace std;

GTEST_TEST(varint, encode) {
    array<byte, 10> a{};
    bytes b = a;

    auto n = varint_coder<pp::uint<1>>::encode(0, b);
    EXPECT_EQ(n.begin() - bytes(a).begin(), 1);
    EXPECT_EQ(a[0], 0_b);

    n = varint_coder<pp::uint<1>>::encode(1, b);
    EXPECT_EQ(n.begin() - b.begin(), 1);
    EXPECT_EQ(a[0], 1_b);

    n = varint_coder<pp::uint<1>>::encode(127, b);
    EXPECT_EQ(n.begin() - b.begin(), 1);
    EXPECT_EQ(a[0], 127_b);

    n = varint_coder<pp::uint<1>>::encode(128, b);
    EXPECT_EQ(n.begin() - b.begin(), 2);
    EXPECT_EQ(a[0], 0x80_b);
    EXPECT_EQ(a[1], 1_b);

    n = varint_coder<pp::uint<2>>::encode(256, b);
    EXPECT_EQ(n.begin() - b.begin(), 2);
    EXPECT_EQ(a[0], 0x80_b);
    EXPECT_EQ(a[1], 2_b);

}

GTEST_TEST(varint, decode) {
    array<byte, 10> a{};
    bytes b = a;

    {
        a = {0_b};
        auto [l, r] = varint_coder<pp::uint<1>>::decode(b);
        EXPECT_EQ(l, 0);
        EXPECT_EQ(r.begin() - b.begin(), 1);
    }

    {
        a = {1_b};
        auto [l, r] = varint_coder<pp::uint<1>>::decode(b);
        EXPECT_EQ(l, 1);
        EXPECT_EQ(r.begin() - b.begin(), 1);
    }

    {
        a = {127_b};
        auto [l, r] = varint_coder<pp::uint<1>>::decode(b);
        EXPECT_EQ(l, 127);
        EXPECT_EQ(r.begin() - b.begin(), 1);
    }

    {
        a = {0x80_b, 1_b};
        auto [l, r] = varint_coder<pp::uint<1>>::decode(b);
        EXPECT_EQ(l, 128);
        EXPECT_EQ(r.begin() - b.begin(), 2);
    }

    {
        a = {1_b};
        auto [l, r] = varint_coder<pp::uint<8>>::decode(b);
        EXPECT_EQ(l, 1);
        EXPECT_EQ(r.begin() - b.begin(), 1);
    }

    {
        a = {4_b};
        auto [l, r] = varint_coder<pp::uint<8>>::decode(b);
        EXPECT_EQ(l, 4);
        EXPECT_EQ(r.begin() - b.begin(), 1);
    }
}
