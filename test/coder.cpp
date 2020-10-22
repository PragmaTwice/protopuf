#include <gtest/gtest.h>

#include <protopuf/coder.h>
#include <protopuf/varint.h>
#include <protopuf/int.h>
#include <protopuf/zigzag.h>
#include <protopuf/array.h>
#include <protopuf/float.h>

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
}
