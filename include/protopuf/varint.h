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

    /// @brief A @ref coder for variable-length integers
    ///
    /// Each byte in a varint, except the last byte, has the most significant bit (msb) set, 
    /// which indicates that there are further bytes to come. 
    /// The lower 7 bits of each byte are used to store the the number in groups of 7 bits, least significant group first.
    ///
    /// References:
    /// 1. https://developers.google.com/protocol-buffers/docs/encoding#varints
    /// 2. https://en.wikipedia.org/wiki/Variable-length_quantity#Applications_and_history
    /// 3. https://en.wikipedia.org/wiki/LEB128
    // template <pp::integral> // failed to pass GCC, strangely
    template <typename>
    class varint_coder;

    template<std::unsigned_integral T>
    class varint_coder<T> {
    public:
        using value_type = T;

        varint_coder() = delete;

        template<coder_mode Mode>
        static constexpr encode_result<Mode> encode(T n, bytes s) {
            auto iter = s.begin();
            const auto end = s.end();

            do {
                if constexpr (Mode::need_checks) {
                    if (!Mode::check_iterator(iter, end)) {
                        return {};
                    }
                }

                *iter = 0b1000'0000_b | std::byte(n);
                n >>= 7, ++iter;
            } while(n != 0);

            *(iter - 1) &= 0b0111'1111_b;

            return Mode::template make_result<encode_result<Mode>>(iter, s.end());
        }

        template<coder_mode Mode>
        static constexpr decode_result<T, Mode> decode(bytes s) {
            T n = 0;

            auto iter = s.begin();
            const auto end = s.end();

            if constexpr (Mode::need_checks) {
                if (!Mode::check_iterator(iter, end)) {
                    return {};
                }
            }

            std::size_t i = 0;
            while((*iter >> 7) == 1_b) {
                n |= static_cast<T>(static_cast<T>(*iter & 0b0111'1111_b) << 7*i);
                ++iter, ++i;

                if constexpr (Mode::need_checks) {
                    if (!Mode::check_iterator(iter, end)) {
                        return {};
                    }
                }
            }
            n |= static_cast<T>(static_cast<T>(*iter++) << 7 * i);

            return Mode::template make_result<decode_result<T, Mode>>(std::move(n), bytes{iter, s.end()});
        }
    };

    template<std::signed_integral T>
    class varint_coder<T> {
    public:
        using value_type = T;

        varint_coder() = delete;

        template<coder_mode Mode>
        static constexpr encode_result<Mode> encode(T n, bytes s) {
            return varint_coder<std::make_unsigned_t<T>>::template encode<Mode>(n, s);
        }

        template<coder_mode Mode>
        static constexpr decode_result<T, Mode> decode(bytes s) {
            return varint_coder<std::make_unsigned_t<T>>::template decode<Mode>(s);
        }
    };


}

#endif //PROTOPUF_VARINT_H
