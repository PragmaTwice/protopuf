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

#ifndef PROTOPUF_FIXED_STRING_H
#define PROTOPUF_FIXED_STRING_H

#include "constant.h"

#include <cstddef>
#include <algorithm>
#include <string_view>

namespace pp {

    /// @brief A fixed string type, which can be put into non-type template parameters (NTTP).
    /// 
    /// Reference:
    /// - http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0732r0.pdf
    template <typename CharT, std::size_t N>
    struct basic_fixed_string
    {
        /// Construct the string from `const CharT[N]`
        constexpr basic_fixed_string(const CharT (&foo)[N]) {
            std::copy(foo, foo + N, data);
        }

        /// The element type of the string
        using value_type = CharT;

        /// The length of the string
        static constexpr auto size = N;

        /// Type cast to `std::basic_string_view<CharT>`
        constexpr operator std::basic_string_view<CharT>() const {
            return data;
        };

        CharT data[N];
    };

    template <typename CharT, std::size_t N, std::size_t M>
    constexpr bool operator==(const basic_fixed_string<CharT, N> &l, const basic_fixed_string<CharT, M> &r) {
        if (N != M) {
            return false;
        }

        for (std::size_t i = 0; i < N; ++i) {
            if(l.data[i] != r.data[i]) {
                return false;
            }
        }

        return true;
    }

    template <typename CharT, std::size_t N, std::size_t M>
    constexpr auto operator<=>(const basic_fixed_string<CharT, N> &l, const basic_fixed_string<CharT, M> &r) {
        for (std::size_t i = 0; i < std::min(N, M); ++i) {
            if(l.data[i] != r.data[i]) {
                return l.data[i] <=> r.data[i];
            }
        }

        return N <=> M;
    }

    /// Type alias for `basic_fixed_string<char, N>` where `N` is length of the string
    template <std::size_t N>
    using fixed_string = basic_fixed_string<char, N>;

    /// Expand the elements of @ref basic_fixed_string to NTTPs, i.e. `expand_fixed_string<"hello">` as `constant_array<char, 'h', 'e', 'l', 'l', 'o', 0>`
    template <basic_fixed_string S>
    constexpr auto expand_fixed_string = []<std::size_t... I>(std::index_sequence<I...>) {
        return constant_array<typename decltype(S)::value_type, S.data[I]...>{};
    }(std::make_index_sequence<decltype(S)::size>{});

    /// @ref basic_fixed_string constant literals, i.e. `"hello"_f` as `constant<basic_fixed_string<char, 6>{"hello"}>`
    template <basic_fixed_string S>
    constexpr auto operator ""_f() {
        return constant<S>{};
    }
}

#endif //PROTOPUF_FIXED_STRING_H
