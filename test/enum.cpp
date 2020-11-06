#include <gtest/gtest.h>

#include <protopuf/byte.h>
#include <protopuf/enum.h>
#include <array>

using namespace pp;
using namespace std;

GTEST_TEST(enum_coder, encode) {
    array<byte, 10> a{};

    enum E1 { red, green, blue = 128 };

    EXPECT_EQ(begin_diff(enum_coder<E1>::encode(green, a), a), 1);
    EXPECT_EQ(a[0], 1_b);

    EXPECT_EQ(begin_diff(enum_coder<E1>::encode(blue, a), a), 2);
    EXPECT_EQ(a[0], 0x80_b);
    EXPECT_EQ(a[1], 0x1_b);

    enum class E2 { red, green, blue };

    EXPECT_EQ(begin_diff(enum_coder<E2>::encode(E2::green, a), a), 1);
    EXPECT_EQ(a[0], 1_b);

    enum E3 : pp::uint<8> { x, y, z };

    EXPECT_EQ(begin_diff(enum_coder<E3>::encode(z, a), a), 1);
    EXPECT_EQ(a[0], 2_b);
}

GTEST_TEST(enum_coder, decode) {
    array<byte, 10> a{};

    enum E1 { red, green, blue = 128 };

    {
        a[0] = 1_b;
        auto [v, n] = enum_coder<E1>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 1);
        EXPECT_EQ(v, green);
    }

    {
        a[0] = 0x80_b; a[1] = 0x1_b;
        auto [v, n] = enum_coder<E1>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 2);
        EXPECT_EQ(v, blue);
    }

    enum class E2 { red, green, blue };

    {
        a[0] = 0x1_b;
        auto [v, n] = enum_coder<E2>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 1);
        EXPECT_EQ(v, E2::green);
    }

    enum E3 : pp::uint<8> { x, y, z };

    {
        a[0] = 0x2_b;
        auto [v, n] = enum_coder<E3>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 1);
        EXPECT_EQ(v, z);
    }
}
