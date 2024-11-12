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

#include <protopuf/array.h>
#include <protopuf/zigzag.h>
#include <array>

#include "test_fixture.h"

using namespace pp;
using namespace std;

template<typename T>
struct test_array_coder : test_fixture<T> {};
TYPED_TEST_SUITE(test_array_coder, coder_mode_types, test_name_generator);

TYPED_TEST(test_array_coder, encode) {
    vector<sint_zigzag<8>> con{sint_zigzag<8>(-1), sint_zigzag<8>(100000), sint_zigzag<8>(9), sint_zigzag<8>(4)};
    array<byte, 10> a{};
    array<byte, 10> e{0x06_b, 0x01_b, 0xC0_b, 0x9A_b, 0x0C_b, 0x12_b, 0x08_b};

    bytes n;
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        array_coder<varint_coder<sint_zigzag<8>>>::encode<typename TestFixture::mode>(con, a), n));
    EXPECT_EQ(begin_diff(n, a), 7);
    EXPECT_EQ(a, e);
}

TYPED_TEST(test_array_coder, decode) {
    vector<sint_zigzag<8>> con{sint_zigzag<8>(-1), sint_zigzag<8>(100000), sint_zigzag<8>(9), sint_zigzag<8>(4)};
    array<byte, 10> a{0x06_b, 0x01_b, 0xC0_b, 0x9A_b, 0x0C_b, 0x12_b, 0x08_b};
    decode_value<vector<sint_zigzag<8>>> value;
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        array_coder<varint_coder<sint_zigzag<8>>>::decode<typename TestFixture::mode>(a), value));
    const auto& [v, n] = value;
    EXPECT_EQ(begin_diff(n, a), 7);
    EXPECT_EQ(con, v);
}

template<typename T>
struct test_string_coder : test_fixture<T> {};
TYPED_TEST_SUITE(test_string_coder, coder_mode_types, test_name_generator);

TYPED_TEST(test_string_coder, encode) {
    array<byte, 10> e{3_b, 0x61_b, 0x62_b, 0x63_b};
    array<byte, 10> a{};
    bytes n;
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        string_coder::encode<typename TestFixture::mode>("abc"s, a), n));
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, e);
}

TYPED_TEST(test_string_coder, decode) {
    auto e = "abc"s;
    array<byte, 10> a{3_b, 0x61_b, 0x62_b, 0x63_b};
    decode_value<std::string> value;
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        string_coder::decode<typename TestFixture::mode>(a), value));
    const auto& [v, n] = value;
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(v, e);
}

GTEST_TEST(array_coder, encode_with_insufficient_buffer_size) {
    const vector<sint_zigzag<8>> con{sint_zigzag<8>(-1), sint_zigzag<8>(100000), sint_zigzag<8>(9), sint_zigzag<8>(4)};
    run_safe_encode_tests_with_insufficient_buffer_size<array_coder<varint_coder<sint_zigzag<8>>>, 7>(con);
}

GTEST_TEST(array_coder, decode_with_insufficient_buffer_size) {
    array<byte, 7> a{0x06_b, 0x01_b, 0xC0_b, 0x9A_b, 0x0C_b, 0x12_b, 0x08_b};
    run_safe_decode_tests_with_insufficient_buffer_size<array_coder<varint_coder<sint_zigzag<8>>>>(a);
}

GTEST_TEST(string_coder, encode_with_insufficient_buffer_size) {
    run_safe_encode_tests_with_insufficient_buffer_size<string_coder, 4>("abc"s);
}

GTEST_TEST(string_coder, decode_with_insufficient_buffer_size) {
    array<byte, 4> a{3_b, 0x61_b, 0x62_b, 0x63_b};
    run_safe_decode_tests_with_insufficient_buffer_size<string_coder>(a);
}