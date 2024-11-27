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

#include <protopuf/int.h>
#include <protopuf/byte.h>

#include <array>
#include <algorithm>

#include "test_fixture.h"

using namespace pp;
using namespace std;

GTEST_TEST(static, int) {
    static_assert(is_same_v<sint<1>, int8_t>);
    static_assert(is_same_v<sint<2>, int16_t>);
    static_assert(is_same_v<sint<4>, int32_t>);
    static_assert(is_same_v<sint<8>, int64_t>);

    static_assert(is_same_v<pp::uint<1>, uint8_t>);
    static_assert(is_same_v<pp::uint<2>, uint16_t>);
    static_assert(is_same_v<pp::uint<4>, uint32_t>);
    static_assert(is_same_v<pp::uint<8>, uint64_t>);
}

array a1{0b101010_b};
array a2{0b1011100_b, 0b1001_b};
array a3{0b0_b, 0b11_b, 0b1111_b, 0b111111_b};

pp::uint<1> u1 = 0b101010;
pp::uint<2> u2 = 0b1001'0101'1100;
pp::uint<4> u3 = 0b00111111'00001111'00000011'00000000;

GTEST_TEST(converter, byte_to_int) {
    EXPECT_EQ(bytes_to_int(span(a1)), u1);
    EXPECT_EQ(bytes_to_int(span(a2)), u2);
    EXPECT_EQ(bytes_to_int(span(a3)), u3);
}

GTEST_TEST(converter, int_to_byte) {
    EXPECT_EQ(int_to_bytes<1>(u1), a1);
    EXPECT_EQ(int_to_bytes<2>(u2), a2);
    EXPECT_EQ(int_to_bytes<4>(u3), a3);
}

template<typename T>
struct test_integer_coder : test_fixture<T> {};
TYPED_TEST_SUITE(test_integer_coder, coder_mode_types, test_name_generator);

array a4{0b101010_b, 0b1011100_b, 0b1001_b, 0b0_b, 0b11_b, 0b1111_b, 0b111111_b};

TYPED_TEST(test_integer_coder, encode) {
    array<byte, 1024> a{};
    span<byte> s = a;

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        integer_coder<pp::uint<1>>::encode<typename TestFixture::mode>(u1, s), s));
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        integer_coder<pp::uint<2>>::encode<typename TestFixture::mode>(u2, s), s));
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        integer_coder<pp::uint<4>>::encode<typename TestFixture::mode>(u3, s), s)); 

    span b = span(a).template subspan<0,7>();

    EXPECT_TRUE(equal(b.begin(), b.end(), a4.begin()));
}

TYPED_TEST(test_integer_coder, decode) {
    span<byte> s = a4;

    {
        pp::uint<1> b1;
        decode_value<pp::uint<1>> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            integer_coder<pp::uint<1>>::decode<typename TestFixture::mode>(s), value));
        tie(b1, s) = value;
        EXPECT_EQ(u1, b1);
    }

    {   
        pp::uint<2> b2;
        decode_value<pp::uint<2>> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            integer_coder<pp::uint<2>>::decode<typename TestFixture::mode>(s), value));
        tie(b2, s) = value;
        EXPECT_EQ(u2, b2);
    }

    {
        pp::uint<4> b3;
        decode_value<pp::uint<4>> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            integer_coder<pp::uint<4>>::decode<typename TestFixture::mode>(s), value));
        tie(b3, s) = value;
        EXPECT_EQ(u3, b3);
    }
}

TYPED_TEST(test_integer_coder, signed) {
    sint<4> m1 = -1;
    array<byte, 4> am1{};
    bytes b;

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        integer_coder<sint<4>>::encode<typename TestFixture::mode>(m1, span(am1)), b));

    EXPECT_EQ(am1[0], 0xff_b);
    EXPECT_EQ(am1[1], 0xff_b);
    EXPECT_EQ(am1[2], 0xff_b);
    EXPECT_EQ(am1[3], 0xff_b);

    decode_value<sint<4>> value;
     ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            integer_coder<sint<4>>::decode<typename TestFixture::mode>(span(am1)), value));
    auto [m1e, _] = value;
    EXPECT_EQ(m1e, m1);
}

GTEST_TEST(integer_coder, encode_with_insufficient_buffer_size) {
    run_safe_encode_tests_with_insufficient_buffer_size<integer_coder<sint<4>>, 4>(-1);
}

GTEST_TEST(integer_coder, decode_with_insufficient_buffer_size) {
    array<byte, 4> a{0xff_b, 0xff_b, 0xff_b, 0xff_b};
    run_safe_decode_tests_with_insufficient_buffer_size<integer_coder<sint<4>>>(a);
}