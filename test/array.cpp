#include <gtest/gtest.h>

#include <protopuf/array.h>
#include <protopuf/zigzag.h>

using namespace pp;
using namespace std;

GTEST_TEST(array_coder, encode) {
    vector<sint_zigzag<8>> con{sint_zigzag<8>(-1), sint_zigzag<8>(100000), sint_zigzag<8>(9), sint_zigzag<8>(4)};
    array<byte, 10> a{};
    array<byte, 10> e{4_b, 1_b, 0xC0_b, 0x9A_b, 0x0C_b, 0x12_b, 0x08_b};
    auto n = array_coder<varint_coder<sint_zigzag<8>>>::encode(con, a);
    EXPECT_EQ(n, 7);
    EXPECT_EQ(a, e);
}
