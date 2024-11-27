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

#include <protopuf/varint.h>
#include <array>

#include "test_fixture.h"

using namespace pp;
using namespace std;

template<typename T>
struct test_varint : test_fixture<T> {};
TYPED_TEST_SUITE(test_varint, coder_mode_types, test_name_generator);

TYPED_TEST(test_varint, encode) {
    array<byte, 10> a{};
    bytes b = a;
    bytes n;

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        varint_coder<pp::uint<1>>::encode<typename TestFixture::mode>(0, b), n));
    EXPECT_EQ(begin_diff(n, a), 1);
    EXPECT_EQ(a[0], 0_b);

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        varint_coder<pp::uint<1>>::encode<typename TestFixture::mode>(1, b), n));
    EXPECT_EQ(begin_diff(n, b), 1);
    EXPECT_EQ(a[0], 1_b);

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        varint_coder<pp::uint<1>>::encode<typename TestFixture::mode>(127, b), n));
    EXPECT_EQ(begin_diff(n, b), 1);
    EXPECT_EQ(a[0], 127_b);

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        varint_coder<pp::uint<1>>::encode<typename TestFixture::mode>(128, b), n));
    EXPECT_EQ(begin_diff(n, b), 2);
    EXPECT_EQ(a[0], 0x80_b);
    EXPECT_EQ(a[1], 1_b);

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        varint_coder<pp::uint<2>>::encode<typename TestFixture::mode>(256, b), n));
    EXPECT_EQ(begin_diff(n, b), 2);
    EXPECT_EQ(a[0], 0x80_b);
    EXPECT_EQ(a[1], 2_b);

}

TYPED_TEST(test_varint, decode) {
    array<byte, 10> a{};
    bytes b = a;

    {
        a = {0_b};
        decode_value<pp::uint<1>> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            varint_coder<pp::uint<1>>::decode<typename TestFixture::mode>(b), value));
        auto [l, r] = value;
        EXPECT_EQ(l, 0);
        EXPECT_EQ(begin_diff(r, b), 1);
    }

    {
        a = {1_b};
        decode_value<pp::uint<1>> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            varint_coder<pp::uint<1>>::decode<typename TestFixture::mode>(b), value));
        auto [l, r] = value;
        EXPECT_EQ(l, 1);
        EXPECT_EQ(begin_diff(r, b), 1);
    }

    {
        a = {127_b};
        decode_value<pp::uint<1>> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            varint_coder<pp::uint<1>>::decode<typename TestFixture::mode>(b), value));
        auto [l, r] = value;
        EXPECT_EQ(l, 127);
        EXPECT_EQ(begin_diff(r, b), 1);
    }

    {
        a = {0x80_b, 1_b};
        decode_value<pp::uint<1>> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            varint_coder<pp::uint<1>>::decode<typename TestFixture::mode>(b), value));
        auto [l, r] = value;
        EXPECT_EQ(l, 128);
        EXPECT_EQ(begin_diff(r, b), 2);
    }

    {
        a = {1_b};
        decode_value<pp::uint<8>> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            varint_coder<pp::uint<8>>::decode<typename TestFixture::mode>(b), value));
        auto [l, r] = value;
        EXPECT_EQ(l, 1);
        EXPECT_EQ(begin_diff(r, b), 1);
    }

    {
        a = {4_b};
        decode_value<pp::uint<8>> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            varint_coder<pp::uint<8>>::decode<typename TestFixture::mode>(b), value));
        auto [l, r] = value;
        EXPECT_EQ(l, 4);
        EXPECT_EQ(begin_diff(r, b), 1);
    }
}

GTEST_TEST(varint_coder, encode_with_insufficient_buffer_size) {
    run_safe_encode_tests_with_insufficient_buffer_size<varint_coder<pp::uint<2>>, 2>(pp::uint<2>(256));
}

GTEST_TEST(varint_coder, decode_with_insufficient_buffer_size) {
    array<byte, 2> a{0x80_b, 1_b};
    run_safe_decode_tests_with_insufficient_buffer_size<varint_coder<pp::uint<1>>>(a);
}