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

#ifndef PROTOPUF_FLOAT_H
#define PROTOPUF_FLOAT_H

#include <concepts>
#include <version>

#if defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L
#include <bit>
#endif

#include "coder.h"
#include "int.h"

namespace pp {

    /// A @ref coder for floating point types
    template<std::floating_point T>
    struct float_coder {
        using value_type = T;

    private:
        using underlying_type = uint<sizeof(T)>;

        static constexpr underlying_type underlying_cast(value_type t) {
        #if defined(FLOAT_CAST_UB_IMPL) || !(__cpp_lib_bit_cast >= 201806L)
            return *reinterpret_cast<underlying_type*>(&t);
        #else
            return std::bit_cast<underlying_type>(t);
        #endif
        }

        static constexpr value_type value_cast(underlying_type t) {
        #if defined(FLOAT_CAST_UB_IMPL) || !(__cpp_lib_bit_cast >= 201806L)
            return *reinterpret_cast<value_type*>(&t);
        #else
            return std::bit_cast<value_type>(t);
        #endif
        }

    public:
        float_coder() = delete;

        template<coder_mode Mode>
        static constexpr encode_result<Mode> encode(T v, bytes b) {
            return integer_coder<underlying_type>::template encode<Mode>(underlying_cast(v), b);
        }

        template<coder_mode Mode>
        static constexpr decode_result<T, Mode> decode(bytes b) {
            decode_value<underlying_type> decode_v;
            if (Mode::get_value_from_result(integer_coder<underlying_type>::template decode<Mode>(b), decode_v)) {
                return Mode::template make_result<decode_result<T, Mode>>(value_cast(decode_v.first), decode_v.second);
            }
            
            return {};
        }
    };

    template <std::size_t N>
    struct floating_impl;

    template <>
    struct floating_impl<4> {
        using type = float;
    };

    template <>
    struct floating_impl<8> {
        using type = double;
    };

    /// Type alias of floating points, i.e. `floating<4>` as `float`, `floating<8>` as `double`
    template <std::size_t N>
    using floating = typename floating_impl<N>::type;

    /// A `std::floating_point<T>` where byte size of `T` equals to `N` 
    template <typename T, std::size_t N>
    concept sized_floating_point = std::floating_point<T> && sizeof(T) == N;

    /// A concept statisfied while `T = float`
    template <typename T>
    concept floating_point32 = sized_floating_point<T, 4>;

    /// A concept statisfied while `T = double`
    template <typename T>
    concept floating_point64 = sized_floating_point<T, 8>;
}

#endif //PROTOPUF_FLOAT_H
