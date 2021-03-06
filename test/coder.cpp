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

#include <protopuf/coder.h>
#include <protopuf/varint.h>
#include <protopuf/int.h>
#include <protopuf/zigzag.h>
#include <protopuf/array.h>
#include <protopuf/float.h>
#include <protopuf/message.h>

using namespace pp;
using namespace std;

GTEST_TEST(static, coder) {
    static_assert(coder<integer_coder<pp::uint<1>>>);
    static_assert(coder<integer_coder<pp::uint<4>>>);
    static_assert(coder<integer_coder<sint<1>>>);
    static_assert(coder<integer_coder<sint<4>>>);
    static_assert(coder<integer_coder<sint_zigzag<1>>>);
    static_assert(coder<integer_coder<sint_zigzag<4>>>);

    static_assert(coder<varint_coder<pp::uint<1>>>);
    static_assert(coder<varint_coder<pp::uint<4>>>);
    static_assert(coder<varint_coder<sint<1>>>);
    static_assert(coder<varint_coder<sint<4>>>);
    static_assert(coder<varint_coder<sint_zigzag<1>>>);
    static_assert(coder<varint_coder<sint_zigzag<4>>>);

    static_assert(coder<array_coder<integer_coder<sint<1>>>>);
    static_assert(coder<array_coder<integer_coder<pp::uint<4>>>>);
    static_assert(coder<array_coder<varint_coder<sint<2>>>>);
    static_assert(coder<array_coder<varint_coder<sint_zigzag<4>>>>);

    static_assert(coder<string_coder>);
    static_assert(coder<basic_string_coder<wchar_t>>);
    static_assert(coder<bytes_coder>);

    static_assert(coder<float_coder<floating<4>>>);
    static_assert(coder<float_coder<floating<8>>>);

    static_assert(coder<message_coder<message<integer_field<"", 1, int>, floating_field<"", 3, float>>>>);
    static_assert(coder<message_coder<message<integer_field<"", 1, int>, string_field<"", 2>, floating_field<"", 4, float>, varint_field<"", 100, sint_zigzag<4>>>> >);
    static_assert(coder<message_coder<message<integer_field<"", 1, int, repeated>, floating_field<"", 0, float, repeated>>>>);

    static_assert(coder<embedded_message_coder<message<integer_field<"", 1, int>, floating_field<"", 3, float>>>>);
    static_assert(coder<embedded_message_coder<message<integer_field<"", 1, int>, string_field<"", 2>, floating_field<"", 4, float>, varint_field<"", 100, sint_zigzag<4>>>> >);
    static_assert(coder<embedded_message_coder<message<integer_field<"", 1, int, repeated>, floating_field<"", 0, float, repeated>>>>);

    static_assert(coder<bool_coder>);

    enum E{};
    static_assert(coder<enum_coder<E>>);
}
