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

#include <protopuf/fixed_string.h>

using namespace pp;
using namespace std;

GTEST_TEST(fixed_string, static) {
    static_assert("x"_f() == "x"_f());
    static_assert("xxx"_f() == "xxx"_f());
    static_assert("x"_f() != "y"_f());
    static_assert("x"_f() != "xx"_f());
    static_assert("x"_f() < "y"_f());
    static_assert("x"_f() < "xy"_f());
    static_assert("x"_f() < "xa"_f());
    static_assert("xy"_f() < "y"_f());
    static_assert("xaa"_f() < "y"_f());

    static_assert(is_same_v<decltype(expand_fixed_string<"hello">), const constant_array<char, 'h', 'e', 'l', 'l', 'o', 0>>);
    static_assert(expand_fixed_string<"hello">.get<2>() == 'l' && expand_fixed_string<"hello">.get<3>() == 'l');
}
