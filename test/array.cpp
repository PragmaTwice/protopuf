//   Copyright 2020 PragmaTwice
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include <gtest/gtest.h>

#include <protopuf/array.h>
#include <protopuf/zigzag.h>

using namespace pp;
using namespace std;

GTEST_TEST(array_coder, encode) {
    vector<sint_zigzag<8>> con{sint_zigzag<8>(-1), sint_zigzag<8>(100000), sint_zigzag<8>(9), sint_zigzag<8>(4)};
    array<byte, 10> a{};
    array<byte, 10> e{0x06_b, 0x01_b, 0xC0_b, 0x9A_b, 0x0C_b, 0x12_b, 0x08_b};
    auto n = array_coder<varint_coder<sint_zigzag<8>>>::encode(con, a);
    EXPECT_EQ(begin_diff(n, a), 7);
    EXPECT_EQ(a, e);
}

GTEST_TEST(array_coder, decode) {
    vector<sint_zigzag<8>> con{sint_zigzag<8>(-1), sint_zigzag<8>(100000), sint_zigzag<8>(9), sint_zigzag<8>(4)};
    array<byte, 10> a{0x06_b, 0x01_b, 0xC0_b, 0x9A_b, 0x0C_b, 0x12_b, 0x08_b};
    auto [v, n] = array_coder<varint_coder<sint_zigzag<8>>>::decode(a);
    EXPECT_EQ(begin_diff(n, a), 7);
    EXPECT_EQ(con, v);
}

GTEST_TEST(string_coder, encode) {
    array<byte, 10> e{3_b, 0x61_b, 0x62_b, 0x63_b};
    array<byte, 10> a{};
    auto n = string_coder::encode("abc"s, a);
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, e);
}

GTEST_TEST(string_coder, decode) {
    auto e = "abc"s;
    array<byte, 10> a{3_b, 0x61_b, 0x62_b, 0x63_b};
    auto [v, n] = string_coder::decode(a);
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(v, e);
}
