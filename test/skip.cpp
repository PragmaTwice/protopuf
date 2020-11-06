#include <gtest/gtest.h>

#include <protopuf/skip.h>
#include <protopuf/array.h>

using namespace pp;
using namespace std;

GTEST_TEST(skipper, static) {
    static_assert(skipper_c<skipper<integer_coder<pp::uint<1>>>>);
    static_assert(skipper_c<skipper<integer_coder<pp::uint<4>>>>);
    static_assert(skipper_c<skipper<integer_coder<sint<1>>>>);
    static_assert(skipper_c<skipper<integer_coder<sint<4>>>>);
    static_assert(skipper_c<skipper<integer_coder<sint_zigzag<1>>>>);
    static_assert(skipper_c<skipper<integer_coder<sint_zigzag<4>>>>);

    static_assert(skipper_c<skipper<varint_coder<pp::uint<1>>>>);
    static_assert(skipper_c<skipper<varint_coder<pp::uint<4>>>>);
    static_assert(skipper_c<skipper<varint_coder<sint<1>>>>);
    static_assert(skipper_c<skipper<varint_coder<sint<4>>>>);
    static_assert(skipper_c<skipper<varint_coder<sint_zigzag<1>>>>);
    static_assert(skipper_c<skipper<varint_coder<sint_zigzag<4>>>>);

    static_assert(skipper_c<skipper<array_coder<integer_coder<sint<1>>>>>);
    static_assert(skipper_c<skipper<array_coder<integer_coder<pp::uint<4>>>>>);
    static_assert(skipper_c<skipper<array_coder<varint_coder<sint<2>>>>>);
    static_assert(skipper_c<skipper<array_coder<varint_coder<sint_zigzag<4>>>>>);

    static_assert(skipper_c<skipper<string_coder>>);
    static_assert(skipper_c<skipper<basic_string_coder<wchar_t>>>);
    static_assert(skipper_c<skipper<bytes_coder>>);

    static_assert(skipper_c<skipper<float_coder<floating<4>>>>);
    static_assert(skipper_c<skipper<float_coder<floating<8>>>>);

    static_assert(skipper_c<skipper<bool_coder>>);

    enum E {};
    static_assert(skipper_c<skipper<enum_coder<E>>>);
}

GTEST_TEST(skipper, encode) {
    EXPECT_EQ(skipper<integer_coder<int>>::encode_skip(0), sizeof(int));
    EXPECT_EQ(skipper<integer_coder<int>>::encode_skip(-1), sizeof(int));
    EXPECT_EQ(skipper<float_coder<float>>::encode_skip(0.123e4), sizeof(float));
    EXPECT_EQ(skipper<varint_coder<int>>::encode_skip(0), 1);
    EXPECT_EQ(skipper<varint_coder<int>>::encode_skip(127), 1);
    EXPECT_EQ(skipper<varint_coder<int>>::encode_skip(128), 2);
    EXPECT_EQ(skipper<varint_coder<int>>::encode_skip(12345), 2);
    EXPECT_EQ(skipper<varint_coder<int>>::encode_skip(123456), 3);
    EXPECT_EQ(skipper<varint_coder<int>>::encode_skip(12345678), 4);
    EXPECT_EQ(skipper<varint_coder<int>>::encode_skip(-1), 5);
    EXPECT_EQ(skipper<varint_coder<int>>::encode_skip(-123456), 5);
    EXPECT_EQ(skipper<string_coder>::encode_skip("twice"), 1 + 5);

    string str = "twice!!!", lstr;
    for(int i = 0; i < 16; ++i) lstr.append(str);
    EXPECT_EQ(skipper<string_coder>::encode_skip(lstr), 2 + 8 * 16);

    wstring wstr = L"twice!!!", lwstr;
    for(int i = 0; i < 30000; ++i) lwstr.append(wstr);
    EXPECT_EQ(skipper<basic_string_coder<wchar_t>>::encode_skip(lwstr), 3 + 4 * 8 * 30000);
}

GTEST_TEST(skipper, decode) {
    {
        array<byte, 10> a{};

        EXPECT_EQ(begin_diff(skipper<integer_coder<int>>::decode_skip(a), a), sizeof(int));
        EXPECT_EQ(begin_diff(skipper<float_coder<float>>::decode_skip(a), a), sizeof(float));
        EXPECT_EQ(begin_diff(skipper<varint_coder<int>>::decode_skip(a), a), 1);
    }

    {
        array<byte, 10> a{0x80_b, 0x80_b, 0x00_b, 0x00_b};

        EXPECT_EQ(begin_diff(skipper<integer_coder<int>>::decode_skip(a), a), sizeof(int));
        EXPECT_EQ(begin_diff(skipper<float_coder<float>>::decode_skip(a), a), sizeof(float));
        EXPECT_EQ(begin_diff(skipper<varint_coder<int>>::decode_skip(a), a), 3);
    }

    {
        array<byte, 200> a{0x80_b, 0x01_b};

        EXPECT_EQ(begin_diff(skipper<string_coder>::decode_skip(a), a), 2 + 128);
    }
}
