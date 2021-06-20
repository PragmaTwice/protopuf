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
            return *(underlying_type*)&t;
        }

        static constexpr value_type value_cast(underlying_type t) {
            return *(value_type*)&t;
        }

    public:
        float_coder() = delete;

        static constexpr bytes encode(T v, bytes b) {
            return integer_coder<underlying_type>::encode(underlying_cast(v), b);
        }

        static constexpr decode_result<T> decode(bytes b) {
            auto p = integer_coder<underlying_type>::decode(b);

            return {value_cast(p.first), p.second};
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
