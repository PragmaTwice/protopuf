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

#ifndef PROTOPUF_SKIP_H
#define PROTOPUF_SKIP_H

#include "coder.h"
#include "int.h"
#include "varint.h"
#include "zigzag.h"
#include "float.h"
#include "bool.h"
#include "enum.h"

namespace pp {

    template <typename T>
    concept encode_skipper = coder<typename T::coder> && requires(typename T::value_type v) {
        { T::encode_skip(v) } -> std::same_as<std::size_t>;
    };

    template <typename T>
    concept decode_skipper = coder<typename T::coder> && requires(bytes v) {
        { T::decode_skip(v) } -> std::same_as<bytes>;
    };

    template <typename T>
    concept skipper_c = encode_skipper<T> && decode_skipper<T>;

    template <coder C>
    struct skipper;

    template <typename T>
    struct skipper<integer_coder<T>> {
        using coder = integer_coder<T>;
        using value_type = T;

        static constexpr std::size_t encode_skip(T) {
            return sizeof(T);
        }

        static constexpr bytes decode_skip(bytes b) {
            return b.subspan<sizeof(T)>();
        }
    };

    template <typename T>
    struct skipper<float_coder<T>> {
        using coder = float_coder<T>;
        using value_type = T;

        static constexpr std::size_t encode_skip(T) {
            return sizeof(T);
        }

        static constexpr bytes decode_skip(bytes b) {
            return b.subspan<sizeof(T)>();
        }
    };

    template <std::unsigned_integral T>
    struct skipper<varint_coder<T>> {
        using coder = varint_coder<T>;
        using value_type = T;

        static constexpr std::size_t encode_skip(T v) {
            std::size_t n = 0;

            do {
                v >>= 7, ++n;
            } while(v != 0);

            return n;
        }

        static constexpr bytes decode_skip(bytes b) {
            auto iter = b.begin();
            while((*iter++ >> 7) == 1_b) {}

            return {iter, b.end()};
        }
    };

    template <std::signed_integral T>
    struct skipper<varint_coder<T>> {
        using coder = varint_coder<T>;
        using value_type = T;

        static constexpr std::size_t encode_skip(T v) {
            return skipper<varint_coder<std::make_unsigned_t<T>>>::encode_skip(v);
        }

        static constexpr bytes decode_skip(bytes b) {
            return skipper<varint_coder<std::make_unsigned_t<T>>>::decode_skip(b);
        }
    };

    template <std::size_t N>
    struct skipper<varint_coder<sint_zigzag<N>>> {
    private:
        using T = sint_zigzag<N>;

    public:
        using coder = varint_coder<T>;
        using value_type = T;

        static constexpr std::size_t encode_skip(T v) {
            return skipper<varint_coder<uint<N>>>::encode_skip(v.get_underlying());
        }

        static constexpr bytes decode_skip(bytes b) {
            return skipper<varint_coder<uint<N>>>::decode_skip(b);
        }
    };

    template <>
    struct skipper<bool_coder> {
    public:
        using coder = bool_coder;
        using value_type = bool;

        static constexpr std::size_t encode_skip(bool v) {
            return skipper<integer_coder<uint<1>>>::encode_skip(v);
        }

        static constexpr bytes decode_skip(bytes b) {
            return skipper<integer_coder<uint<1>>>::decode_skip(b);
        }
    };

    template <typename T>
    struct skipper<enum_coder<T>> {
    public:
        using coder = enum_coder<T>;
        using value_type = T;

        static constexpr std::size_t encode_skip(T v) {
            return skipper<varint_coder<std::underlying_type_t<T>>>::encode_skip(static_cast<std::underlying_type_t<T>>(v));
        }

        static constexpr bytes decode_skip(bytes b) {
            return skipper<integer_coder<std::underlying_type_t<T>>>::decode_skip(b);
        }
    };

}

#endif //PROTOPUF_SKIP_H
