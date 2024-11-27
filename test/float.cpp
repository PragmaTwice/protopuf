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

#include <protopuf/float.h>
#include <array>

#include "test_fixture.h"

using namespace pp;
using namespace std;

template<typename T>
struct test_float_coder : test_fixture<T> {};
TYPED_TEST_SUITE(test_float_coder, coder_mode_types, test_name_generator);

TYPED_TEST(test_float_coder, encode) {
    array<byte, 4> a{};
    bytes n;

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        float_coder<float>::encode<typename TestFixture::mode>(0, a), n));
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, (array{0_b, 0_b, 0_b, 0_b}));

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        float_coder<float>::encode<typename TestFixture::mode>(1, a), n));
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, (array{0_b, 0_b, 0x80_b, 0x3f_b}));

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        float_coder<float>::encode<typename TestFixture::mode>(-1, a), n));
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, (array{0_b, 0_b, 0x80_b, 0xbf_b}));

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        float_coder<float>::encode<typename TestFixture::mode>(1.234f, a), n));
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, (array{0xb6_b, 0xf3_b, 0x9d_b, 0x3f_b}));

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        float_coder<float>::encode<typename TestFixture::mode>(1.234e5, a), n));
    EXPECT_EQ(begin_diff(n, a), 4);
    EXPECT_EQ(a, (array{0_b, 0x04_b, 0xf1_b, 0x47_b}));
}

TYPED_TEST(test_float_coder, decoder) {
    decode_value<float> value;

    array a{0_b, 0_b, 0_b, 0_b};
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        float_coder<float>::decode<typename TestFixture::mode>(a), value));
    auto [v, n] = value;
    EXPECT_EQ(v, 0.);
    EXPECT_EQ(begin_diff(n, a), 4);

    a = {0_b, 0_b, 0x80_b, 0x3f_b};
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        float_coder<float>::decode<typename TestFixture::mode>(a), value));
    tie(v, n) = value;
    EXPECT_FLOAT_EQ(v, 1);

    a = {0_b, 0_b, 0x80_b, 0xbf_b};
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        float_coder<float>::decode<typename TestFixture::mode>(a), value));
    tie(v, n) = value;
    EXPECT_FLOAT_EQ(v, -1);

    a = {0xb6_b, 0xf3_b, 0x9d_b, 0x3f_b};
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        float_coder<float>::decode<typename TestFixture::mode>(a), value));
    tie(v, n) = value;
    EXPECT_FLOAT_EQ(v, 1.234f);

    a = {0_b, 0x04_b, 0xf1_b, 0x47_b};
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        float_coder<float>::decode<typename TestFixture::mode>(a), value));
    tie(v, n) = value;
    EXPECT_FLOAT_EQ(v, 1.234e5);
}

GTEST_TEST(float_coder, encode_with_insufficient_buffer_size) {
    run_safe_encode_tests_with_insufficient_buffer_size<float_coder<float>, 4>(1.234f);
}

GTEST_TEST(float_coder, decode_with_insufficient_buffer_size) {
    array<byte, 4> a{0xb6_b, 0xf3_b, 0x9d_b, 0x3f_b};
    run_safe_decode_tests_with_insufficient_buffer_size<float_coder<float>>(a);
}