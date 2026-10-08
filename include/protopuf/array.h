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

#ifndef PROTOPUF_ARRAY_H
#define PROTOPUF_ARRAY_H

#include <ranges>
#include <string>
#include <vector>
#include "coder.h"
#include "varint.h"
#include "skip.h"

namespace pp {

    /// @brief A `std::ranges::range<T>` where some element can be inserted into `T`, 
    /// as type `T` has member function `insert` with `T::iterator` and `T::value_type` as parameters.
    template <typename T>
    concept insertable_range = std::ranges::range<T> && requires (T t) {
        sizeof(std::inserter(t, t.end()));
    };

    /// @brief A @ref insertable_range where the parameter type `T` satisfies `std::ranges::sized_range`.
    template <typename T>
    concept insertable_sized_range = insertable_range<T> && std::ranges::sized_range<T>;

    /// @brief A @ref coder for range types, i.e. `std::vector<T>`.
    ///
    /// @param C the @ref coder for the element type of the range types, i.e. `C = integer_coder<int>` for `R = std::vector<int>`
    /// @param R the range type to encode/decode, which should satisfy `std::ranges::sized_range`
    template <coder C, std::ranges::sized_range R = std::vector<typename C::value_type>>
    struct array_coder {
        using value_type = R;

        array_coder() = delete;

        template <coder_mode Mode = safe_mode>
        static constexpr encode_result<Mode> encode(const R& con, bytes b) {
            uint<8> n = 0;

            for(const auto &i : con) {
                n += skipper<C>::encode_skip(i);
            }

            bytes safe_b;
            if (!Mode::get_value_from_result(varint_coder<uint<8>>::encode<Mode>(n, b), safe_b)) {
                return {};
            }

            for(const auto& i : con) {
                if (!Mode::get_value_from_result(C::template encode<Mode>(i, safe_b), safe_b)) {
                    return {};
                }
            }
            return encode_result<Mode>{safe_b};
        }

        template <coder_mode Mode = safe_mode>
        static constexpr decode_result<R, Mode> decode(bytes b) {
            decode_value<uint<8>> decode_len;
            if (!Mode::get_value_from_result(varint_coder<uint<8>>::decode<Mode>(b), decode_len)) {
                return {};
            }

            uint<8> len = 0;
            std::tie(len, b) = decode_len;
            R con;

            if constexpr (requires { con.reserve(); }) {
                con.reserve(len);
            }

            decode_value<typename C::value_type> decode_v;
            while(len) {
                if (Mode::get_value_from_result(C::template decode<Mode>(b), decode_v)) {
                    std::tie(*std::inserter(con, con.end()), b) = std::move(decode_v);
                    --len;
                } else {
                    return {};
                }
            }

            return Mode::template make_result<decode_result<R, Mode>>(std::move(con), b);
        }
    };

    template <coder C, typename R>
    struct skipper<array_coder<C, R>> {
        using coder = array_coder<C, R>;
        using value_type = R;

        static constexpr std::size_t encode_skip(const R &con) {
            uint<8> n = 0;

            for(const auto &i : con) {
                n += skipper<C>::encode_skip(i);
            }

            n += skipper<varint_coder<uint<8>>>::encode_skip(n);

            return n;
        }

        template <coder_mode Mode = safe_mode>
        static constexpr decode_skip_result<Mode> decode_skip(bytes b) {
            decode_value<uint<8>> decode_len;
            if (!Mode::get_value_from_result(varint_coder<uint<8>>::decode<Mode>(b), decode_len)) {
                return {};
            }

            uint<8> n = 0;
            std::tie(n, b) = decode_len;

            if constexpr (Mode::need_checks) {
                if (!Mode::check_bytes_span(b, n)) {
                    return {};
                }
            }

            return Mode::template make_result<decode_skip_result<Mode>>(b.subspan(n));
        }
    };

    /// Type alias of @ref coder for `std::basic_string<T>`
    template <integral T>
    using basic_string_coder = array_coder<integer_coder<T>, std::basic_string<T>>;

    /// Type alias of @ref coder for `std::string`
    using string_coder = basic_string_coder<std::string::value_type>;

    /// Type alias of @ref coder for `std::vector<uint<1>>`
    using bytes_coder = array_coder<integer_coder<uint<1>>>;

}

#endif //PROTOPUF_ARRAY_H
