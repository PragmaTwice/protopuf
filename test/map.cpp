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
#include <array>

#include "test_fixture.h"

using namespace pp;
using namespace std;

template<typename T>
struct test_map : test_fixture<T> {};
TYPED_TEST_SUITE(test_map, coder_mode_types, test_name_generator);

using StrIntMap = message<map_field<"map", 233, string_coder, varint_coder<int>>>;

TYPED_TEST(test_map, encode) {
    StrIntMap map{ {{"b", 2}, {"a", 1}, {"c", 3}} };
    array<byte, 30> buffer{};
    bytes b;

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        message_coder<StrIntMap>::encode<typename TestFixture::mode>(map, buffer), b));

    array<byte, 30> buffer_exp { 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x61_b, 0x10_b, 0x01_b, 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x62_b, 0x10_b, 0x02_b, 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x63_b, 0x10_b, 0x03_b };
    EXPECT_EQ(buffer, buffer_exp);
}

TYPED_TEST(test_map, decode) {
    array<byte, 30> buffer { 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x61_b, 0x10_b, 0x01_b, 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x62_b, 0x10_b, 0x02_b, 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x63_b, 0x10_b, 0x03_b };
    
    decode_value<StrIntMap> value;
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
       message_coder<StrIntMap>::decode<typename TestFixture::mode>(buffer), value));
    auto [map, _] = value;
    ASSERT_EQ(map["map"_f].count("a"), 1);
    ASSERT_EQ(map["map"_f].count("b"), 1);
    ASSERT_EQ(map["map"_f].count("c"), 1);
    EXPECT_EQ(map["map"_f].at("a"), 1);
    EXPECT_EQ(map["map"_f].at("b"), 2);
    EXPECT_EQ(map["map"_f].at("c"), 3);
}

GTEST_TEST(test_map, utility) {
    using Msg = message<map_field<"data", 11, string_coder, varint_coder<int>>>;

    Msg msg1 {{{"one", 1}, {"two", 2}, {"three", 3}}}, msg2 {{{"four", 4}, {"five", 5}}};
    
    msg1.merge(msg2);

    EXPECT_EQ(msg1["data"_f].size(), 5);
    EXPECT_EQ(msg1["data"_f], (map<optional<string>, optional<int>>{{"one", 1}, {"two", 2}, {"three", 3}, {"four", 4}, {"five", 5}}));
}

GTEST_TEST(map_coder, encode_with_insufficient_buffer_size) {
    const StrIntMap map{ {{"b", 2}, {"a", 1}, {"c", 3}} };
    run_safe_encode_tests_with_insufficient_buffer_size<message_coder<StrIntMap>, 24>(map);
}

GTEST_TEST(map_coder, decode_with_insufficient_buffer_size) {
    array<byte, 24> a { 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x61_b, 0x10_b, 0x01_b, 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x62_b, 0x10_b, 0x02_b, 0xca_b, 0x0e_b, 0x05_b, 0x0a_b, 0x01_b, 0x63_b, 0x10_b, 0x03_b };
    run_safe_decode_test_with_insufficient_buffer_size<message_coder<StrIntMap>, 5>(a);
    run_safe_decode_test_with_insufficient_buffer_size<message_coder<StrIntMap>, 15>(a);
    run_safe_decode_test_with_insufficient_buffer_size<message_coder<StrIntMap>, 17>(a);
    run_safe_decode_test_with_insufficient_buffer_size<message_coder<StrIntMap>, 20>(a);
    run_safe_decode_test_with_insufficient_buffer_size<message_coder<StrIntMap>, 21>(a);
}