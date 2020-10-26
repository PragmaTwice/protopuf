#include <gtest/gtest.h>

#include <protopuf/skip.h>
#include <protopuf/array.h>

using namespace pp;
using namespace std;

GTEST_TEST(skipper, static) {
    static_assert(skipper_c<skipper<integer_coder<pp::uint<1>>>>);
    static_assert(skipper_c<skipper<integer_coder<pp::uint<4>>>>);
    static_assert(skipper_c<skipper<integer_coder<sint<1>>>>);
    static_assert(skipper_c<skipper<integer_coder<sint<4>>>>);
    static_assert(skipper_c<skipper<integer_coder<sint_zigzag<1>>>>);
    static_assert(skipper_c<skipper<integer_coder<sint_zigzag<4>>>>);

    static_assert(skipper_c<skipper<varint_coder<pp::uint<1>>>>);
    static_assert(skipper_c<skipper<varint_coder<pp::uint<4>>>>);
    static_assert(skipper_c<skipper<varint_coder<sint<1>>>>);
    static_assert(skipper_c<skipper<varint_coder<sint<4>>>>);
    static_assert(skipper_c<skipper<varint_coder<sint_zigzag<1>>>>);
    static_assert(skipper_c<skipper<varint_coder<sint_zigzag<4>>>>);

    static_assert(skipper_c<skipper<array_coder<integer_coder<sint<1>>>>>);
    static_assert(skipper_c<skipper<array_coder<integer_coder<pp::uint<4>>>>>);
    static_assert(skipper_c<skipper<array_coder<varint_coder<sint<2>>>>>);
    static_assert(skipper_c<skipper<array_coder<varint_coder<sint_zigzag<4>>>>>);

    static_assert(skipper_c<skipper<string_coder>>);
    static_assert(skipper_c<skipper<basic_string_coder<wchar_t>>>);
    static_assert(skipper_c<skipper<bytes_coder>>);

    static_assert(skipper_c<skipper<float_coder<floating<4>>>>);
    static_assert(skipper_c<skipper<float_coder<floating<8>>>>);
}
