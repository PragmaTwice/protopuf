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

#include <cstddef>
#include <algorithm>

namespace pp {

    template <typename CharT, std::size_t N>
    struct basic_fixed_string
    {
        constexpr basic_fixed_string(const CharT (&foo)[N]) {
            std::copy(foo, foo + N, data);
        }

        using value_type = CharT;
        static constexpr auto size = N;

        CharT data[N];
    };

    template <typename CharT, std::size_t N, std::size_t M>
    constexpr auto operator==(const basic_fixed_string<CharT, N> &l, const basic_fixed_string<CharT, M> &r) {
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
    constexpr auto operator!=(const basic_fixed_string<CharT, N> &l, const basic_fixed_string<CharT, M> &r) {
        return !(l == r);
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

    template <std::size_t N>
    using fixed_string = basic_fixed_string<char, N>;

    template<auto v>
    struct constant {
        static constexpr auto value = v;

        using value_type = decltype(v);
        using type = constant;

        constexpr operator value_type() const noexcept { return value; }
        constexpr value_type operator()() const noexcept { return value; }
    };

    template <std::size_t, auto...>
    struct constant_get_impl;

    template <std::size_t N, auto v1, auto... vn>
    struct constant_get_impl<N, v1, vn...> {
        static constexpr auto value = constant_get_impl<N - 1, vn...>::value;
    };

    template <auto v1, auto... vn>
    struct constant_get_impl<0, v1, vn...> {
        static constexpr auto value = v1;
    };

    template <std::size_t N, auto... v>
    constexpr auto constant_get = constant_get_impl<N, v...>::value;

    template <auto... v>
    struct constant_tuple {
        using type = constant_tuple;

        static constexpr auto size = sizeof...(v);

        template <std::size_t N>
        static constexpr auto value = constant_get<N, v...>;

        template <std::size_t N>
        using value_type = decltype(value<N>);

        template <std::size_t N>
        constexpr decltype(auto) get() const {
            return constant<constant_get<N, v...>>{};
        }
    };

    template <typename T, T... v>
    struct constant_array {
        using type = constant_array;

        static constexpr auto size = sizeof...(v);

        template <std::size_t N>
        static constexpr auto value = constant_get<N, v...>;

        using value_type = T;

        template <std::size_t N>
        constexpr decltype(auto) get() const {
            return constant<constant_get<N, v...>>{};
        }
    };

    template <basic_fixed_string S>
    constexpr auto expand_fixed_string = []<std::size_t... I>(std::index_sequence<I...>) {
        return constant_array<typename decltype(S)::value_type, S.data[I]...>{};
    }(std::make_index_sequence<decltype(S)::size>{});

    template <basic_fixed_string S>
    constexpr auto operator ""_f() {
        return constant<S>{};
    }
}

#endif //PROTOPUF_FIXED_STRING_H
