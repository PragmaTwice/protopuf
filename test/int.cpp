#include <gtest/gtest.h>

#include <protopuf/int.h>
#include <protopuf/byte.h>

#include <algorithm>

using namespace pp;
using namespace std;

GTEST_TEST(static, int) {
    static_assert(is_same_v<sint<1>, int8_t>);
    static_assert(is_same_v<sint<2>, int16_t>);
    static_assert(is_same_v<sint<4>, int32_t>);
    static_assert(is_same_v<sint<8>, int64_t>);

    static_assert(is_same_v<pp::uint<1>, uint8_t>);
    static_assert(is_same_v<pp::uint<2>, uint16_t>);
    static_assert(is_same_v<pp::uint<4>, uint32_t>);
    static_assert(is_same_v<pp::uint<8>, uint64_t>);
}

array a1{0b101010_b};
array a2{0b1011100_b, 0b1001_b};
array a3{0b0_b, 0b11_b, 0b1111_b, 0b111111_b};

pp::uint<1> u1 = 0b101010;
pp::uint<2> u2 = 0b1001'0101'1100;
pp::uint<4> u3 = 0b00111111'00001111'00000011'00000000;

GTEST_TEST(converter, byte_to_int) {
    EXPECT_EQ(bytes_to_int(span(a1)), u1);
    EXPECT_EQ(bytes_to_int(span(a2)), u2);
    EXPECT_EQ(bytes_to_int(span(a3)), u3);
}

GTEST_TEST(converter, int_to_byte) {
    EXPECT_EQ(int_to_bytes<1>(u1), a1);
    EXPECT_EQ(int_to_bytes<2>(u2), a2);
    EXPECT_EQ(int_to_bytes<4>(u3), a3);
}

array a4{0b101010_b, 0b1011100_b, 0b1001_b, 0b0_b, 0b11_b, 0b1111_b, 0b111111_b};

GTEST_TEST(integer_coder, encode) {
    array<byte, 1024> a;
    span<byte> s = a;

    size_t n1 = integer_coder<pp::uint<1>>::encode(u1, s);
    s = s.subspan(n1);
    size_t n2 = integer_coder<pp::uint<2>>::encode(u2, s);
    s = s.subspan(n2);
    size_t n3 = integer_coder<pp::uint<4>>::encode(u3, s);
    s = s.subspan(n3);

    span a2 = span(a).subspan<0,7>();

    EXPECT_TRUE(equal(a.begin(), a.end(), a2.begin()));
}
