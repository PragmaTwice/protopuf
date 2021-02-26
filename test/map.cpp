//   Copyright 2020-2021 PragmaTwice
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

#include <protopuf/map.h>

using namespace pp;
using namespace std;

using StrIntMap = message<map_field<"map", 233, string_coder, varint_coder<int>>>;

GTEST_TEST(map, encode) {
    StrIntMap map{ {{"b", 2}, {"a", 1}, {"c", 3}} };
    array<byte, 30> buffer{};

    message_coder<StrIntMap>::encode(map, buffer);
    array<byte, 30> buffer_exp { 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x61_b, 0x10_b, 0x01_b, 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x62_b, 0x10_b, 0x02_b, 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x63_b, 0x10_b, 0x03_b };
    EXPECT_EQ(buffer, buffer_exp);
}

GTEST_TEST(map, decode) {
    array<byte, 30> buffer { 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x61_b, 0x10_b, 0x01_b, 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x62_b, 0x10_b, 0x02_b, 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x63_b, 0x10_b, 0x03_b };
    
    auto [map, _] = message_coder<StrIntMap>::decode(buffer);
    EXPECT_EQ(map["map"_f].at("a"), 1);
    EXPECT_EQ(map["map"_f].at("b"), 2);
    EXPECT_EQ(map["map"_f].at("c"), 3);
}
