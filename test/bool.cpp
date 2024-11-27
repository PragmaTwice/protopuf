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

#include <protopuf/bool.h>
#include <array>

#include "test_fixture.h"

using namespace pp;
using namespace std;

template<typename T>
struct test_bool_coder : test_fixture<T> {};
TYPED_TEST_SUITE(test_bool_coder, coder_mode_types, test_name_generator);

TYPED_TEST(test_bool_coder, encode) {
    array<byte, 10> a{};

    bytes n;
    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        bool_coder::encode<typename TestFixture::mode>(true, a), n));
    EXPECT_EQ(begin_diff(n, a), 1);
    EXPECT_EQ(a[0], 1_b);

    ASSERT_TRUE(TestFixture::mode::get_value_from_result(
        bool_coder::encode<typename TestFixture::mode>(false, a), n));
    EXPECT_EQ(begin_diff(n, a), 1);
    EXPECT_EQ(a[0], 0_b);
}

TYPED_TEST(test_bool_coder, decode) {
    array<byte, 10> a{};

    {
        decode_value<bool> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            bool_coder::decode<typename TestFixture::mode>(a), value));
        auto[v, n] = value;
        EXPECT_EQ(begin_diff(n, a), 1);
        EXPECT_EQ(v, false);
    }

    {
        a[0] = 1_b;

        decode_value<bool> value;
        ASSERT_TRUE(TestFixture::mode::get_value_from_result(
            bool_coder::decode<typename TestFixture::mode>(a), value));
        auto[v, n] = value;
        EXPECT_EQ(begin_diff(n, a), 1);
        EXPECT_EQ(v, true);
    }
}
