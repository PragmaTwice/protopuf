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

#include <protopuf/zigzag.h>

#include <array>

#include "test_fixture.h"

using namespace pp;
using namespace std;

GTEST_TEST(static, sint_zigzag) {
    static_assert(pp::is_integral_v<sint_zigzag<1>>);
    static_assert(pp::is_integral_v<sint_zigzag<4>>);
}

GTEST_TEST(sint_zigzag, integer) {
    sint_zigzag<1> z(0), m1(-1), p1(1), m2(-2), p2(2);

    EXPECT_EQ(z.get(), 0);
    EXPECT_EQ(z.get_underlying(), 0);
    EXPECT_EQ(m1.get(), -1);
    EXPECT_EQ(m1.get_underlying(), 1);
    EXPECT_EQ(p1.get(), 1);
    EXPECT_EQ(p1.get_underlying(), 2);
    EXPECT_EQ(m2.get(), -2);
    EXPECT_EQ(m2.get_underlying(), 3);
    EXPECT_EQ(p2.get(), 2);
    EXPECT_EQ(p2.get_underlying(), 4);


    auto uz = sint_zigzag<1>::from_uint(0),
        um1 = sint_zigzag<1>::from_uint(1),
        up1 = sint_zigzag<1>::from_uint(2),
        uz2 = sint_zigzag<1>::from_uint(3),
        up2 = sint_zigzag<1>::from_uint(4);

    EXPECT_EQ(uz.get(), 0);
    EXPECT_EQ(um1.get(), -1);
    EXPECT_EQ(up1.get(), 1);
    EXPECT_EQ(uz2.get(), -2);
    EXPECT_EQ(up2.get(), 2);

    sint_zigzag<4> p10k(10000);
    EXPECT_EQ(p10k.get(), 10000);
    EXPECT_EQ(p10k.get_underlying(), 20000);
}

GTEST_TEST(sint_zigzag, bytes) {
    sint_zigzag<4> p10k(10000);
    array a10k {0x20_b, 0x4e_b, 0_b, 0_b};
    EXPECT_EQ(p10k.dump(), a10k);
}

template<typename T>
struct test_sint_zigzag : test_fixture<T> {};
TYPED_TEST_SUITE(test_sint_zigzag, coder_mode_types, test_name_generator);

TYPED_TEST(test_sint_zigzag, integer_coder) {
    sint_zigzag<4> p10k(10000);
    array a10k {0x20_b, 0x4e_b, 0_b, 0_b};

    array<byte, 4> a10ke {};
    bytes b;
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        integer_coder<sint_zigzag<4>>::encode<typename TestFixture::mode>(p10k, a10ke), b));
    EXPECT_EQ(a10k, a10ke);

    decode_value<sint_zigzag<4>> value;
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        integer_coder<sint_zigzag<4>>::decode<typename TestFixture::mode>(a10k), value));
    auto [p10ke, _] = value;
    EXPECT_EQ(p10k, p10ke);
}

TYPED_TEST(test_sint_zigzag, varint_coder) {
    sint_zigzag<4> p10k(10000);
    array a10k {0xa0_b, 0x9c_b, 0x01_b, 0_b};

    array<byte, 4> a10ke {};
    bytes b;
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        varint_coder<sint_zigzag<4>>::encode<typename TestFixture::mode>(p10k, a10ke), b));
    EXPECT_EQ(a10k, a10ke);

    decode_value<sint_zigzag<4>> value;
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        varint_coder<sint_zigzag<4>>::decode<typename TestFixture::mode>(a10k), value));
    auto [p10ke, _] = value;
    EXPECT_EQ(p10k, p10ke);
}

GTEST_TEST(sint_zigzag_integer_coder, encode_with_insufficient_buffer_size) {
    const sint_zigzag<4> p10k(10000);
    run_safe_encode_tests_with_insufficient_buffer_size<integer_coder<sint_zigzag<4>>, 4>(p10k);
}

GTEST_TEST(sint_zigzag_integer_coder, decode_with_insufficient_buffer_size) {
    array a10k {0x20_b, 0x4e_b, 0_b, 0_b};
    run_safe_decode_tests_with_insufficient_buffer_size<integer_coder<sint_zigzag<4>>>(a10k);
}

GTEST_TEST(sint_zigzag_varint_coder, encode_with_insufficient_buffer_size) {
    const sint_zigzag<4> p10k(10000);
    run_safe_encode_tests_with_insufficient_buffer_size<varint_coder<sint_zigzag<4>>, 3>(p10k);
}

GTEST_TEST(sint_zigzag_varint_coder, decode_with_insufficient_buffer_size) {
    array a10k {0xa0_b, 0x9c_b, 0x01_b};
    run_safe_decode_tests_with_insufficient_buffer_size<varint_coder<sint_zigzag<4>>>(a10k);
}