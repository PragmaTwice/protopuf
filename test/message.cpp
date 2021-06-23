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

#include <protopuf/message.h>
#include <array>

using namespace pp;
using namespace std;

GTEST_TEST(message, function) {
    message<integer_field<"", 1, int>, floating_field<"", 3, float>> m{12, 1.23};
    static_assert(m.size == 2);

    EXPECT_EQ(m.get<1>(), 12);
    EXPECT_FLOAT_EQ(m.get<3>().value(), 1.23);

    auto m2 = m;
    EXPECT_EQ(m2, m);

    auto m5 = std::move(m2);
    EXPECT_EQ(m5, m);

    decltype(m5) m6, m7;
    m6 = m5;
    EXPECT_EQ(m6, m);

    m7 = std::move(m6);
    EXPECT_EQ(m7, m);

    message<integer_field<"", 1, int>, string_field<"", 2>, floating_field<"", 4, float>, varint_field<"", 100, sint_zigzag<4>>> m3{
        12, "345", 6.78, sint_zigzag<4>(90)
    };
    static_assert(m3.size == 4);

    EXPECT_EQ(m3.get<1>(), 12);
    EXPECT_EQ(m3.get<2>(), "345");
    EXPECT_FLOAT_EQ(m3.get<4>().value(), 6.78);
    EXPECT_EQ(m3.get<100>(), sint_zigzag<4>(90));

    message<integer_field<"", 1, int, repeated>, floating_field<"", 0, float, repeated>> m4{{1, 2, 3}, {1., 1.2, 1.23}};
    static_assert(m4.size == 2);

    EXPECT_EQ(m4.get_base<1>(), (vector{1,2,3}));
    EXPECT_EQ(m4.get_base<0>(), (vector{1.f,1.2f,1.23f}));

}

GTEST_TEST(message_coder, encode) {
    {
        message<varint_field<"", 1, int>> m{150};
        array<byte, 10> a{};
        auto n = message_coder<decltype(m)>::encode(m, a);
        EXPECT_EQ(begin_diff(n, a), 3);
        EXPECT_EQ(a, (array<byte, 10>{0x08_b, 0x96_b, 0x01_b}));
    }

    {
        message<integer_field<"", 1, int>, string_field<"", 2>, floating_field<"", 4, float>, varint_field<"", 100, sint_zigzag<4>>> m{
                12, "345", 6.78f, sint_zigzag<4>(90)
        };
        array<byte, 20> a{};
        auto n = message_coder<decltype(m)>::encode(m, a);
        EXPECT_EQ(begin_diff(n, a), 19);
        EXPECT_EQ(a, (array<byte, 20>{0x0d_b, 0x0c_b, 0x00_b, 0x00_b, 0x00_b, 0x12_b, 0x03_b, 0x33_b, 0x34_b, 0x35_b,
                                      0x25_b, 0xc3_b, 0xf5_b, 0xd8_b, 0x40_b, 0xa0_b, 0x06_b, 0xb4_b, 0x01_b}));
    }

    {
        message<integer_field<"", 10, int, repeated>, floating_field<"", 5, float, repeated>> m{{1, 2, 3}, {1.2, 3.4e5}};
        array<byte, 30> a{};
        auto n = message_coder<decltype(m)>::encode(m, a);
        EXPECT_EQ(begin_diff(n, a), 25);
        EXPECT_EQ(a, (array<byte, 30>{0x55_b, 0x01_b, 0x00_b, 0x00_b, 0x00_b, 0x55_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b,
                                           0x55_b, 0x03_b, 0x00_b, 0x00_b, 0x00_b, 0x2d_b, 0x9A_b, 0x99_b, 0x99_b, 0x3f_b,
                                           0x2d_b, 0x00_b, 0x04_b, 0xa6_b, 0x48_b}));
    }

    {
        message<array_field<"", 2, varint_coder<pp::uint<8>>>, string_field<"", 20>> m{vector<pp::uint<8>>{1,123,456789,0}, "hello"};
        array<byte, 30> a{};
        auto n = message_coder<decltype(m)>::encode(m, a);
        EXPECT_EQ(begin_diff(n, a), 16);
        EXPECT_EQ(a, (array<byte, 30>{0x12_b, 0x06_b, 0x01_b, 0x7b_b, 0xd5_b, 0xf0_b, 0x1b_b, 0x00_b, 0xa2_b, 0x01_b,
                                          0x05_b, 0x68_b, 0x65_b, 0x6c_b, 0x6c_b, 0x6f_b}));
    }

}

GTEST_TEST(message_coder, decode) {
    {
        message<varint_field<"", 1, int>> m;
        array<byte, 10> a{0x08_b, 0x96_b, 0x01_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 3);
        EXPECT_EQ(v.get<1>(), 150);
    }

    {
        message<integer_field<"", 1, int>, string_field<"", 2>, floating_field<"", 4, float>, varint_field<"", 100, sint_zigzag<4>>> m;
        array<byte, 20> a{0x0d_b, 0x0c_b, 0x00_b, 0x00_b, 0x00_b, 0x12_b, 0x03_b, 0x33_b, 0x34_b, 0x35_b,
                         0x25_b, 0xc3_b, 0xf5_b, 0xd8_b, 0x40_b, 0xa0_b, 0x06_b, 0xb4_b, 0x01_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 19);
        EXPECT_EQ(v.get<1>(), 12);
        EXPECT_EQ(v.get<2>(), "345");
        EXPECT_FLOAT_EQ(v.get<4>().value(), 6.78f);
        EXPECT_EQ(v.get<100>(), sint_zigzag<4>(90));
    }

    {
        message<string_field<"", 2>, varint_field<"", 100, sint_zigzag<4>>, integer_field<"", 1, int>, floating_field<"", 4, float>> m;
        array<byte, 20> a{0x0d_b, 0x0c_b, 0x00_b, 0x00_b, 0x00_b, 0x12_b, 0x03_b, 0x33_b, 0x34_b, 0x35_b,
                          0x25_b, 0xc3_b, 0xf5_b, 0xd8_b, 0x40_b, 0xa0_b, 0x06_b, 0xb4_b, 0x01_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 19);
        EXPECT_EQ(v.get<1>(), 12);
        EXPECT_EQ(v.get<2>(), "345");
        EXPECT_FLOAT_EQ(v.get<4>().value(), 6.78f);
        EXPECT_EQ(v.get<100>(), sint_zigzag<4>(90));
    }

    {
        message<integer_field<"", 10, int, repeated>, floating_field<"", 5, float, repeated>> m;
        array<byte, 30>a {0x55_b, 0x01_b, 0x00_b, 0x00_b, 0x00_b, 0x55_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b,
                          0x55_b, 0x03_b, 0x00_b, 0x00_b, 0x00_b, 0x2d_b, 0x9A_b, 0x99_b, 0x99_b, 0x3f_b,
                          0x2d_b, 0x00_b, 0x04_b, 0xa6_b, 0x48_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 25);
        EXPECT_EQ(v.get_base<10>(), (vector {1, 2, 3}));
        EXPECT_EQ(v.get_base<5>(), (vector {1.2f, 3.4e5f}));
    }

    {
        message<integer_field<"", 10, int, repeated>, floating_field<"", 5, float, repeated>> m;
        array<byte, 30>a {0x55_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b,
                          0x55_b, 0x03_b, 0x00_b, 0x00_b, 0x00_b, 0x2d_b, 0x9A_b, 0x99_b, 0x99_b, 0x3f_b,
                          0x2d_b, 0x00_b, 0x04_b, 0xa6_b, 0x48_b,0x55_b, 0x01_b, 0x00_b, 0x00_b, 0x00_b };
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 25);
        EXPECT_EQ(v.get_base<10>(), (vector {2, 3, 1}));
        EXPECT_EQ(v.get_base<5>(), (vector {1.2f, 3.4e5f}));
    }

    {
        message<array_field<"", 2, varint_coder<pp::uint<8>>>, string_field<"", 20>> m;
        array<byte, 30> a{0x12_b, 0x06_b, 0x01_b, 0x7b_b, 0xd5_b, 0xf0_b, 0x1b_b, 0x00_b, 0xa2_b, 0x01_b,
                          0x05_b, 0x68_b, 0x65_b, 0x6c_b, 0x6c_b, 0x6f_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 16);
        EXPECT_EQ(v.get_base<2>(), (vector<pp::uint<8>>{1,123,456789,0}));
        EXPECT_EQ(v.get<20>(), "hello");
    }
}

GTEST_TEST(message_coder, decode_with_unknown_fields) {
    {
        message<integer_field<"", 1, int>, string_field<"", 2>, floating_field<"", 4, float>, varint_field<"", 100, sint_zigzag<4>>> m;
        array<byte, 30> a{0x55_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b, 0x0d_b, 0x0c_b, 0x00_b, 0x00_b, 0x00_b,
                          0x12_b, 0x03_b, 0x33_b, 0x34_b, 0x35_b,0x25_b, 0xc3_b, 0xf5_b, 0xd8_b, 0x40_b,
                          0xa0_b, 0x06_b, 0xb4_b, 0x01_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 24);
        EXPECT_EQ(v.get<1>(), 12);
        EXPECT_EQ(v.get<2>(), "345");
        EXPECT_FLOAT_EQ(v.get<4>().value(), 6.78f);
        EXPECT_EQ(v.get<100>(), sint_zigzag<4>(90));
    }

    {
        message<integer_field<"", 1, int>, string_field<"", 2>, floating_field<"", 4, float>, varint_field<"", 100, sint_zigzag<4>>> m;
        array<byte, 40> a{0x55_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b, 0x0d_b, 0x0c_b, 0x00_b, 0x00_b, 0x00_b,
                          0x70_b, 0x81_b,0x82_b,0x83_b,0x84_b,0x85_b,0x86_b,0x87_b,0x88_b,0x09_b,
                          0x12_b, 0x03_b, 0x33_b, 0x34_b, 0x35_b,0x25_b, 0xc3_b, 0xf5_b, 0xd8_b, 0x40_b,
                          0xa0_b, 0x06_b, 0xb4_b, 0x01_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 34);
        EXPECT_EQ(v.get<1>(), 12);
        EXPECT_EQ(v.get<2>(), "345");
        EXPECT_FLOAT_EQ(v.get<4>().value(), 6.78f);
        EXPECT_EQ(v.get<100>(), sint_zigzag<4>(90));
    }

    {
        message<integer_field<"", 1, int>, string_field<"", 2>, floating_field<"", 4, float>, varint_field<"", 100, sint_zigzag<4>>> m;
        array<byte, 50> a{0x55_b, 0x02_b, 0x00_b, 0x00_b, 0x00_b, 0x0d_b, 0x0c_b, 0x00_b, 0x00_b, 0x00_b,
                          0x70_b, 0x81_b,0x82_b,0x83_b,0x84_b,0x85_b,0x86_b,0x87_b,0x88_b,0x09_b,
                          0x12_b, 0x03_b, 0x33_b, 0x34_b, 0x35_b,0x25_b, 0xc3_b, 0xf5_b, 0xd8_b, 0x40_b,
                          0x62_b, 0x07_b, 0x00_b, 0xff_b, 0x00_b, 0xff_b,0x00_b, 0xff_b, 0x00_b,
                          0xa0_b, 0x06_b, 0xb4_b, 0x01_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 43);
        EXPECT_EQ(v.get<1>(), 12);
        EXPECT_EQ(v.get<2>(), "345");
        EXPECT_FLOAT_EQ(v.get<4>().value(), 6.78f);
        EXPECT_EQ(v.get<100>(), sint_zigzag<4>(90));
    }
}

GTEST_TEST(message_coder, nested_encode) {
    {
        message<varint_field<"", 1, int>> m{150};
        message<message_field<"", 3, decltype(m)>> m2(m);
        array<byte, 10> a{};
        auto n = message_coder<decltype(m2)>::encode(m2, a);
        EXPECT_EQ(begin_diff(n, a), 5);
        EXPECT_EQ(a, (array<byte, 10>{0x1a_b, 0x03_b, 0x08_b, 0x96_b, 0x01_b}));
    }

    {
        message<varint_field<"", 1, int>, string_field<"", 3>> m1{150, "alice"},  m2{22, "bob"}, m3{444456, "tom"};
        message<message_field<"", 3, decltype(m1), repeated>, string_field<"", 8>> m({m1, m2, m3}, "class 102");
        array<byte, 50> a{};
        auto n = message_coder<decltype(m)>::encode(m, a);
        EXPECT_EQ(begin_diff(n, a), 43);
        EXPECT_EQ(a, (array<byte, 50>{0x1a_b, 0x0a_b, 0x08_b, 0x96_b, 0x01_b, 0x1a_b, 0x05_b, 0x61_b, 0x6c_b, 0x69_b,
                                      0x63_b, 0x65_b, 0x1a_b, 0x07_b, 0x08_b, 0x16_b, 0x1a_b, 0x03_b, 0x62_b, 0x6f_b,
                                      0x62_b, 0x1a_b, 0x09_b, 0x08_b, 0xa8_b, 0x90_b, 0x1b_b, 0x1a_b, 0x03_b, 0x74_b,
                                      0x6f_b, 0x6d_b, 0x42_b, 0x09_b, 0x63_b, 0x6c_b, 0x61_b, 0x73_b, 0x73_b, 0x20_b,
                                      0x31_b, 0x30_b, 0x32_b}));
    }

    {
        using Student = message<varint_field<"", 1, uint32_t>, string_field<"", 3>>;
        using Class = message<string_field<"", 8>, message_field<"", 3, Student, repeated>>;

        // serialization
        Student twice {123, "twice"}, tom{456, "tom"}, jerry{123456, "jerry"};
        Class myClass {"class 101", {tom, jerry}};
        myClass.get<3>().push_back(twice);

        array<byte, 64> buffer{};
        auto bufferEnd = message_coder<Class>::encode(myClass, buffer);
        EXPECT_EQ(begin_diff(bufferEnd, buffer), 45);

        // deserialization
        auto [yourClass, bufferEnd2] = message_coder<Class>::decode(buffer);
        EXPECT_EQ(yourClass.get<8>(), "class 101");
        EXPECT_EQ(yourClass.get<3>()[2].get<3>(), "twice");
        EXPECT_EQ(yourClass.get<3>()[1], (Student{123456, "jerry"}));
        EXPECT_EQ(yourClass, myClass);
        EXPECT_EQ(begin_diff(bufferEnd2, bufferEnd), 0);
    }

    {
        using Student = message<uint32_field<"", 1>, string_field<"", 3>>;
        using Class = message<string_field<"", 8>, message_field<"", 3, Student, repeated>>;

        // serialization
        Student twice {123, "twice"}, tom{456, "tom"}, jerry{123456, "jerry"};
        Class myClass {"class 101", {tom, jerry}};
        myClass[3_i].push_back(twice);

        array<byte, 64> buffer{};
        auto bufferEnd = message_coder<Class>::encode(myClass, buffer);
        EXPECT_EQ(begin_diff(bufferEnd, buffer), 45);

        // deserialization
        auto [yourClass, bufferEnd2] = message_coder<Class>::decode(buffer);
        EXPECT_EQ(yourClass[8_i], "class 101");
        EXPECT_EQ(yourClass[3_i][2][3_i], "twice");
        EXPECT_EQ(yourClass[3_i][2][1_i], 123);
        EXPECT_EQ(yourClass[3_i][1], (Student{123456, "jerry"}));
        EXPECT_EQ(yourClass, myClass);
        EXPECT_EQ(begin_diff(bufferEnd2, bufferEnd), 0);
    }

    {
        using Student = message<uint32_field<"id", 1>, string_field<"name", 3>>;
        using Class = message<string_field<"name", 8>, message_field<"students", 3, Student, repeated>>;

        // serialization
        Student twice {123, "twice"}, tom{456, "tom"}, jerry{123456, "jerry"};
        Class myClass {"class 101", {tom, jerry}};
        myClass["students"_f].push_back(twice);

        array<byte, 64> buffer{};
        auto bufferEnd = message_coder<Class>::encode(myClass, buffer);
        EXPECT_EQ(begin_diff(bufferEnd, buffer), 45);

        // deserialization
        auto [yourClass, bufferEnd2] = message_coder<Class>::decode(buffer);
        EXPECT_EQ(yourClass["name"_f], "class 101");
        EXPECT_EQ(yourClass["students"_f][2]["name"_f], "twice");
        EXPECT_EQ(yourClass["students"_f][2]["id"_f], 123);
        EXPECT_EQ(yourClass["students"_f][1], (Student{123456, "jerry"}));
        EXPECT_EQ(yourClass, myClass);
        EXPECT_EQ(begin_diff(bufferEnd2, bufferEnd), 0);
    }
}

GTEST_TEST(message_coder, nested_decode) {
    {
        message<varint_field<"", 1, int>> m;
        message<message_field<"", 3, decltype(m)>> m2;
        array<byte, 10> a{0x1a_b, 0x03_b, 0x08_b, 0x96_b, 0x01_b};
        auto[v, n] = message_coder<decltype(m2)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 5);
        EXPECT_EQ(v.get_base<3>().value().get<1>(), 150);
    }

    {
        message<varint_field<"", 1, int>, string_field<"", 3>> m1;
        message<message_field<"", 3, decltype(m1), repeated>, string_field<"", 8>> m;
        array<byte, 50> a{0x1a_b, 0x0a_b, 0x08_b, 0x96_b, 0x01_b, 0x1a_b, 0x05_b, 0x61_b, 0x6c_b, 0x69_b,
                          0x63_b, 0x65_b, 0x1a_b, 0x07_b, 0x08_b, 0x16_b, 0x1a_b, 0x03_b, 0x62_b, 0x6f_b,
                          0x62_b, 0x1a_b, 0x09_b, 0x08_b, 0xa8_b, 0x90_b, 0x1b_b, 0x1a_b, 0x03_b, 0x74_b,
                          0x6f_b, 0x6d_b, 0x42_b, 0x09_b, 0x63_b, 0x6c_b, 0x61_b, 0x73_b, 0x73_b, 0x20_b,
                          0x31_b, 0x30_b, 0x32_b};
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 43);
        EXPECT_EQ(v.get<8>(), "class 102");
        EXPECT_EQ(v.get_base<3>()[0].get<1>(), 150);
        EXPECT_EQ(v.get_base<3>()[0].get<3>(), "alice");
        EXPECT_EQ(v.get_base<3>()[1], (decltype(m1){22, "bob"}));
        EXPECT_EQ(v.get_base<3>()[2], (decltype(m1){444456, "tom"}));
    }

    {
        message<varint_field<"", 1, int>, string_field<"", 3>> m1;
        message<message_field<"", 3, decltype(m1), repeated>, string_field<"", 8>> m;
        array<byte, 50> a{0x1a_b, 0x0a_b, 0x08_b, 0x96_b, 0x01_b, 0x1a_b, 0x05_b, 0x61_b, 0x6c_b, 0x69_b,0x63_b, 0x65_b,
                          0x42_b, 0x09_b, 0x63_b, 0x6c_b, 0x61_b, 0x73_b, 0x73_b, 0x20_b,0x31_b, 0x30_b, 0x32_b,
                          0x1a_b, 0x07_b, 0x08_b, 0x16_b, 0x1a_b, 0x03_b, 0x62_b, 0x6f_b,0x62_b,
                          0x1a_b, 0x09_b, 0x1a_b, 0x03_b, 0x74_b, 0x6f_b, 0x6d_b, 0x08_b, 0xa8_b, 0x90_b, 0x1b_b };
        auto [v, n] = message_coder<decltype(m)>::decode(a);
        EXPECT_EQ(begin_diff(n, a), 43);
        EXPECT_EQ(v.get<8>(), "class 102");
        EXPECT_EQ(v.get_base<3>()[0].get<1>(), 150);
        EXPECT_EQ(v.get_base<3>()[0].get<3>(), "alice");
        EXPECT_EQ(v.get_base<3>()[1], (decltype(m1){22, "bob"}));
        EXPECT_EQ(v.get_base<3>()[2], (decltype(m1){444456, "tom"}));
    }
}

GTEST_TEST(message, fold) {
    message<int32_field<"int", 1>, string_field<"str", 2>> m {1, "hello"};

    auto tup = m.fold([]<typename T, typename U>(T&& x, U&& y){
        return tuple_cat(forward<T>(x), make_tuple(forward<U>(y).cast_to_base()));
    }, tuple{});

    static_assert(is_same_v<decltype(tup), tuple<optional<int32>, optional<string>>>);
    EXPECT_EQ(tup, (tuple{ optional<int32>{1}, optional<string>{"hello"} }));

    auto count = m.fold([](auto x, auto&& y){
        return x + (empty_field(y) ? 0 : 1);
    }, 0);

    EXPECT_EQ(count, 2);

    size_t count2 = 0;
    m.for_each([&count2](auto&& x) {
        count2 += empty_field(x) ? 0 : 1;
    });

    EXPECT_EQ(count2, 2);
}

GTEST_TEST(message, static) {
    static_assert(std::is_same_v<type_forward<int, const float>, const int>);
    static_assert(std::is_same_v<type_forward<int, volatile float>, volatile int>);
    static_assert(std::is_same_v<type_forward<int, const volatile float>, const volatile int>);
    static_assert(std::is_same_v<type_forward<int, float&>, int&>);
    static_assert(std::is_same_v<type_forward<int, float&&>, int&&>);
    static_assert(std::is_same_v<type_forward<int, volatile float&&>, volatile int&&>);
    static_assert(std::is_same_v<type_forward<int, const float&>, const int&>);
}

GTEST_TEST(message, merge) {
    using Person = message<string_field<"name", 11>, string_field<"titles", 22, repeated>, int32_field<"age", 33>>;

    Person john {"John", {"student", "volunteer", "driver"}, 88}, lihua{"Li Hua", {"teacher"}, {}};

    john.merge(lihua);

    EXPECT_EQ(john["name"_f], "Li Hua");
    EXPECT_EQ(john["titles"_f], (vector<string>{"student", "volunteer", "driver", "teacher"}));
    EXPECT_EQ(john["age"_f], 88);

    EXPECT_EQ(lihua["name"_f], "Li Hua");

    
    john.merge(Person{{}, {"unknown"}, 99});
    
    EXPECT_EQ(john["name"_f], "Li Hua");
    EXPECT_EQ(john["titles"_f], (vector<string>{"student", "volunteer", "driver", "teacher", "unknown"}));
    EXPECT_EQ(john["age"_f], 99);

    auto merged = merge(Person{"a", {}, {}}, Person{{}, {"b"}, 123}, Person{{}, {"c", "d"}, 124}, Person{{}, {}, {}});
    EXPECT_EQ(merged["name"_f], "a");
    EXPECT_EQ(merged["titles"_f], (vector<string>{"b", "c", "d"}));
    EXPECT_EQ(merged["age"_f], 124);

    merged = merge(merged, Person{"e", {"f"}, {}});
    EXPECT_EQ(merged["name"_f], "e");
    EXPECT_EQ(merged["titles"_f], (vector<string>{"b", "c", "d", "f"}));
    EXPECT_EQ(merged["age"_f], 124);
}
