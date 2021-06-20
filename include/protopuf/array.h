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

    /// @brief @ref coder for range types, i.e. `std::vector<T>`.
    ///
    /// @param C the @ref coder for the element type of the range types, i.e. `C = integer_coder<int>` for `R = std::vector<int>`
    /// @param R the range type to encode/decode, which should satisfy `std::ranges::sized_range`
    template <coder C, std::ranges::sized_range R = std::vector<typename C::value_type>>
    struct array_coder {
        using value_type = R;

        array_coder() = delete;

        static constexpr bytes encode(const R& con, bytes b) {
            uint<8> n = 0;

            for(const auto &i : con) {
                n += skipper<C>::encode_skip(i);
            }

            b = varint_coder<uint<8>>::encode(n, b);

            for(const auto& i : con) {
                b = C::encode(i, b);
            }

            return b;
        }

        static constexpr decode_result<R> decode(bytes b) {
            uint<8> len = 0;
            std::tie(len, b) = varint_coder<uint<8>>::decode(b);
            R con;

            auto origin_b = b;
            while(begin_diff(b, origin_b) < len) {
                std::tie(*std::inserter(con, con.end()), b) = C::decode(b);
            }

            return {con, b};
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

        static constexpr bytes decode_skip(bytes b) {
            uint<8> n = 0;
            std::tie(n, b) = varint_coder<uint<8>>::decode(b);

            return b.subspan(n);
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
