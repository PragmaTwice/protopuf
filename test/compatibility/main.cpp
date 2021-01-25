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
#include <message.pb.h>

using namespace pp;
using namespace std;

using Student = message<uint32_field<"id", 1>, string_field<"name", 3>>;
using Class = message<string_field<"name", 8>, message_field<"students", 3, Student, repeated>>;

GTEST_TEST(compatibility, encode) {
    Student twice {123, "twice"}, tom{456, "tom"}, jerry{123456, "jerry"};
    Class myClass {"class 101", {tom, jerry, twice}};

    array<byte, 64> buffer{};
    message_coder<Class>::encode(myClass, buffer);

    pb::Class yourClass;
    yourClass.ParseFromArray(buffer.data(), buffer.size());

    EXPECT_EQ(yourClass.name(), "class 101");
    EXPECT_EQ(yourClass.students_size(), 3);
    EXPECT_EQ(yourClass.students(0).id(), 456);
    EXPECT_EQ(yourClass.students(0).name(), "tom");
    EXPECT_EQ(yourClass.students(1).id(), 123456);
    EXPECT_EQ(yourClass.students(1).name(), "jerry");
    EXPECT_EQ(yourClass.students(2).id(), 123);
    EXPECT_EQ(yourClass.students(2).name(), "twice");
}

GTEST_TEST(compatibility, decode) {
    pb::Class yourClass;
    yourClass.set_name("class 101");
    pb::Student* tom = yourClass.add_students();
    tom->set_id(456);
    tom->set_name("tom");
    pb::Student* jerry = yourClass.add_students();
    jerry->set_id(123456);
    jerry->set_name("jerry");
    pb::Student* twice = yourClass.add_students();
    twice->set_id(123);
    twice->set_name("twice");

    array<byte, 64> buffer{};
    yourClass.SerializeToArray(buffer.data(), buffer.size());

    auto [myClass, _] = message_coder<Class>::decode(buffer);
    EXPECT_EQ(myClass["name"_f], "class 101");
    EXPECT_EQ(myClass["students"_f][2]["name"_f], "twice");
    EXPECT_EQ(myClass["students"_f][2]["id"_f], 123);
    EXPECT_EQ(myClass["students"_f][1], (Student{123456, "jerry"}));
    EXPECT_EQ(myClass["students"_f][0], (Student{456, "tom"}));
}
