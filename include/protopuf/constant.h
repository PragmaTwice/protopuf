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

#ifndef PROTOPUF_CONSTANT_H
#define PROTOPUF_CONSTANT_H

#include <cstddef>

namespace pp {
    
    /// A constant type with a constant as NTTP
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

    /// Get element value by index `N` from a NTTP list `v...` 
    template <std::size_t N, auto... v>
    constexpr auto constant_get = constant_get_impl<N, v...>::value;

    template <std::size_t, typename...>
    struct type_get_impl;

    template <std::size_t N, typename T, typename... Ts>
    struct type_get_impl<N, T, Ts...> {
        using type = typename type_get_impl<N - 1, Ts...>::type;
    };

    template <typename T, typename... Ts>
    struct type_get_impl<0, T, Ts...> {
        using type = T;
    };

    /// Get element type by index `N` from a type list `Ts...` 
    template <std::size_t N, typename... Ts>
    using type_get = typename type_get_impl<N, Ts...>::type;

    /// A constant tuple with constants as NTTPs.
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

    /// A constant array with constants of type `T` as NTTPs.
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
    
}

#endif //PROTOPUF_CONSTANT_H