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

#include <cxxabi.h>

using namespace pp;
using namespace std;

GTEST_TEST(reflection, dynamic_get_by_name) {
    using Student = message<uint32_field<"id", 1>, string_field<"name", 3>>;
    using Class = message<string_field<"name", 8>, message_field<"students", 3, Student, repeated>>;

    Student twice {123, "twice"}, tom{456, "tom"}, jerry{123456, "jerry"};
    Class myClass {"class 101", {tom, jerry}};
    myClass["students"_f].push_back(twice);

    EXPECT_TRUE(dynamic_get_by_name(overloaded([](auto&& x){
        if constexpr (remove_reference_t<decltype(x)>::name == "name"_f()) {
            EXPECT_EQ(x.value(), "class 101");
        }
    }), myClass, "name"));

    EXPECT_TRUE(dynamic_get_by_name(overloaded([](auto&& x){
        if constexpr (remove_reference_t<decltype(x)>::name == "name"_f()) {
            return x.value() == "class 101";
        } else {
            return false;
        }
    }), myClass, "name").value());
}
