#include <gtest/gtest.h>

#include <protopuf/varint.h>

using namespace pp;
using namespace std;

GTEST_TEST(varint, encode) {
    array<byte, 10> a{};

    auto n = varint_coder<pp::uint<1>>::encode(0, span(a));
    EXPECT_EQ(n, 1);
    EXPECT_EQ(a[0], 0_b);

    n = varint_coder<pp::uint<1>>::encode(1, span(a));
    EXPECT_EQ(n, 1);
    EXPECT_EQ(a[0], 1_b);

    n = varint_coder<pp::uint<1>>::encode(127, span(a));
    EXPECT_EQ(n, 1);
    EXPECT_EQ(a[0], 127_b);

    n = varint_coder<pp::uint<1>>::encode(128, span(a));
    EXPECT_EQ(n, 2);
    EXPECT_EQ(a[0], 0x80_b);
    EXPECT_EQ(a[1], 1_b);

    n = varint_coder<pp::uint<2>>::encode(256, span(a));
    EXPECT_EQ(n, 2);
    EXPECT_EQ(a[0], 0x80_b);
    EXPECT_EQ(a[1], 2_b);

}

GTEST_TEST(varint, decode) {
    array<byte, 10> a{0_b};

    auto p = varint_coder<pp::uint<1>>::decode(span(a));
    EXPECT_EQ(p, decode_result<pp::uint<1>>(0, 1));

    a = {1_b};
    p = varint_coder<pp::uint<1>>::decode(span(a));
    EXPECT_EQ(p, decode_result<pp::uint<1>>(1, 1));

    a = {127_b};
    p = varint_coder<pp::uint<1>>::decode(span(a));
    EXPECT_EQ(p, decode_result<pp::uint<1>>(127, 1));

    a = {0x80_b, 1_b};
    p = varint_coder<pp::uint<1>>::decode(span(a));
    EXPECT_EQ(p, decode_result<pp::uint<1>>(128, 1));
}
