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

#include <protopuf/reflection.h>

using namespace pp;
using namespace std;

GTEST_TEST(reflection, dynamic_visit_by_name) {
    using Student = message<uint32_field<"id", 1>, string_field<"name", 3>>;
    using Class = message<string_field<"name", 8>, message_field<"students", 3, Student, repeated>>;

    Student twice {123, "twice"}, tom{456, "tom"}, jerry{123456, "jerry"};
    const Class myClass {"class 101", {tom, jerry, twice}};

    EXPECT_TRUE(dynamic_visit_by_name([](auto&& x){
        if constexpr (remove_reference_t<decltype(x)>::name == "name"_f()) {
            EXPECT_EQ(x.value(), "class 101");
        } else {
            FAIL();
        }
    }, myClass, "name"));

    EXPECT_TRUE(dynamic_visit_by_name([](auto&& x){
        if constexpr (remove_reference_t<decltype(x)>::name == "name"_f()) {
            return x.value() == "class 101";
        } else {
            return false;
        }
    }, myClass, "name").value_or(false));

    EXPECT_TRUE(dynamic_visit_by_name([](auto&& x){
        if constexpr (remove_reference_t<decltype(x)>::name == "students"_f()) {
            EXPECT_EQ(x.size(), 3);
        } else {
            FAIL();
        }
    }, myClass, "students"));

    auto f = overloaded{[](auto&&) {
        FAIL();
    }, [](const Class::get_type_by_name<"name">& x){
        EXPECT_EQ(x.value(), "class 101");
    }, [](const Class::get_type_by_name<"students">& x){
        EXPECT_EQ(x.size(), 3);
    }};

    EXPECT_TRUE(dynamic_visit_by_name(f, myClass, "name"));
    EXPECT_TRUE(dynamic_visit_by_name(f, myClass, "students"));

    EXPECT_FALSE(dynamic_visit_by_name([](auto&&){}, myClass, "unknown"));
    EXPECT_FALSE(dynamic_visit_by_name([](auto&&){ return 0; }, myClass, "unknown"));

    EXPECT_TRUE(dynamic_visit_by_number(f, myClass, 3));
    EXPECT_TRUE(dynamic_visit_by_number(f, myClass, 8));

    EXPECT_FALSE(dynamic_visit_by_number(f, myClass, 999));

    using Message = message<int32_field<"int", 22>, string_field<"str", 33>>;

    Message myMsg {12, "hell"};

    auto g = overloaded{[](auto&&) {
        FAIL();
    }, [](Message::get_type_by_name<"str">& x){
        x.value() += "o";
    }, [](Message::get_type_by_name<"int">& x){
        x.value() *= 10;
        x.value() += 3;
    }};

    dynamic_visit_by_name(g, myMsg, "str");
    EXPECT_EQ(myMsg["str"_f], "hello");
    EXPECT_EQ(myMsg["int"_f], 12);

    dynamic_visit_by_name(g, myMsg, "int");
    EXPECT_EQ(myMsg["str"_f], "hello");
    EXPECT_EQ(myMsg["int"_f], 123);

    dynamic_visit_by_number(g, myMsg, 33);
    EXPECT_EQ(myMsg["str"_f], "helloo");
    EXPECT_EQ(myMsg["int"_f], 123);

    dynamic_visit_by_number(g, myMsg, 22);
    EXPECT_EQ(myMsg["str"_f], "helloo");
    EXPECT_EQ(myMsg["int"_f], 1233);
}
