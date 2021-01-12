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

#ifndef PROTOPUF_VARINT_H
#define PROTOPUF_VARINT_H

#include <concepts>
#include "int.h"
#include "byte.h"
#include "coder.h"

namespace pp {

    // varint encoder/decoder
    // template <pp::integral> // failed to pass GCC, strangely
    template <typename>
    class varint_coder;

    template<std::unsigned_integral T>
    class varint_coder<T> {
    public:
        using value_type = T;

        varint_coder() = delete;

        static constexpr bytes encode(T n, bytes s) {
            auto iter = s.begin();
            do {
                *iter = 0b1000'0000_b | std::byte(n);
                n >>= 7, ++iter;
            } while(n != 0);

            *(iter - 1) &= 0b0111'1111_b;

            return {iter, s.end()};
        }

        static constexpr decode_result<T> decode(bytes s) {
            T n = 0;

            auto iter = s.begin();
            std::size_t i = 0;
            while((*iter >> 7) == 1_b) {
                n |= static_cast<T>(*iter & 0b0111'1111_b) << 7*i;
                ++iter, ++i;
            }
            n |= static_cast<T>(*iter++) << 7 * i;

            return {n, {iter, s.end()}};
        }
    };

    template<std::signed_integral T>
    class varint_coder<T> {
    public:
        using value_type = T;

        varint_coder() = delete;

        static constexpr bytes encode(T n, bytes s) {
            return varint_coder<std::make_unsigned_t<T>>::encode(n, s);
        }

        static constexpr decode_result<T> decode(bytes s) {
            return varint_coder<std::make_unsigned_t<T>>::decode(s);
        }
    };


}

#endif //PROTOPUF_VARINT_H
