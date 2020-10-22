#include <gtest/gtest.h>

#include <protopuf/message.h>

using namespace pp;
using namespace std;

GTEST_TEST(message, function) {
    message<integer_field<1, int>, float_field<3, float>> m{12, 1.23};
    static_assert(m.size == 2);

    EXPECT_EQ(m.get<1>(), 12);
    EXPECT_FLOAT_EQ(m.get<3>().value(), 1.23);

    auto m2 = m;
    EXPECT_EQ(m2, m);

    message<integer_field<1, int>, string_field<2>, float_field<4, float>, varint_field<100, sint_zigzag<4>>> m3{
        12, "345", 6.78, sint_zigzag<4>(90)
    };
    static_assert(m3.size == 4);

    EXPECT_EQ(m3.get<1>(), 12);
    EXPECT_EQ(m3.get<2>(), "345");
    EXPECT_FLOAT_EQ(m3.get<4>().value(), 6.78);
    EXPECT_EQ(m3.get<100>(), sint_zigzag<4>(90));

    message<integer_field<1, int, repeated>, float_field<0, float, repeated>> m4{{1,2,3}, {1.,1.2,1.23}};
    static_assert(m4.size == 2);

    EXPECT_EQ(m4.get_base<1>(), (vector{1,2,3}));
    EXPECT_EQ(m4.get_base<0>(), (vector{1.f,1.2f,1.23f}));

}

GTEST_TEST(message_coder, encode) {
    {
        message<varint_field<1, int>> m{150};
        array<byte, 10> a{};
        auto n = message_coder<decltype(m)>::encode(m, a);
        EXPECT_EQ(n.begin() - bytes(a).begin(), 3);
        EXPECT_EQ(a, (array<byte, 10>{0x08_b, 0x96_b, 0x01_b}));
    }

    {
        message<integer_field<1, int>, string_field<2>, float_field<4, float>, varint_field<100, sint_zigzag<4>>> m{
                12, "345", 6.78f, sint_zigzag<4>(90)
        };
        array<byte, 20> a{};
        auto n = message_coder<decltype(m)>::encode(m, a);
        EXPECT_EQ(n.begin() - bytes(a).begin(), 19);
        EXPECT_EQ(a, (array<byte, 20>{0x0d_b, 0x0c_b, 0x00_b, 0x00_b, 0x00_b, 0x12_b, 0x03_b, 0x33_b, 0x34_b, 0x35_b,
                                      0x25_b, 0xc3_b, 0xf5_b, 0xd8_b, 0x40_b, 0xa0_b, 0x06_b, 0xb4_b, 0x01_b}));
    }

}

GTEST_TEST(message_coder, decode) {
    {
        message<varint_field<1, int>> m;
        array<byte, 10> a{0x08_b, 0x96_b, 0x01_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(n.begin() - bytes(a).begin(), 3);
        EXPECT_EQ(v.get<1>(), 150);
    }

    {
        message<integer_field<1, int>, string_field<2>, float_field<4, float>, varint_field<100, sint_zigzag<4>>> m;
        array<byte, 20> a{0x0d_b, 0x0c_b, 0x00_b, 0x00_b, 0x00_b, 0x12_b, 0x03_b, 0x33_b, 0x34_b, 0x35_b,
                         0x25_b, 0xc3_b, 0xf5_b, 0xd8_b, 0x40_b, 0xa0_b, 0x06_b, 0xb4_b, 0x01_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(n.begin() - bytes(a).begin(), 19);
        EXPECT_EQ(v.get<1>(), 12);
        EXPECT_EQ(v.get<2>(), "345");
        EXPECT_FLOAT_EQ(v.get<4>().value(), 6.78f);
        EXPECT_EQ(v.get<100>(), sint_zigzag<4>(90));
    }

    {
        message<string_field<2>, varint_field<100, sint_zigzag<4>>, integer_field<1, int>, float_field<4, float>> m;
        array<byte, 20> a{0x0d_b, 0x0c_b, 0x00_b, 0x00_b, 0x00_b, 0x12_b, 0x03_b, 0x33_b, 0x34_b, 0x35_b,
                          0x25_b, 0xc3_b, 0xf5_b, 0xd8_b, 0x40_b, 0xa0_b, 0x06_b, 0xb4_b, 0x01_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(n.begin() - bytes(a).begin(), 19);
        EXPECT_EQ(v.get<1>(), 12);
        EXPECT_EQ(v.get<2>(), "345");
        EXPECT_FLOAT_EQ(v.get<4>().value(), 6.78f);
        EXPECT_EQ(v.get<100>(), sint_zigzag<4>(90));
    }
}
