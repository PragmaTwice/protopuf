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

#ifndef PROTOPUF_MESSAGE_H
#define PROTOPUF_MESSAGE_H

#include "coder.h"
#include "field.h"
#include "float.h"

#include <unordered_map>
#include <functional>

namespace pp {

    template <typename...>
    constexpr bool are_same = true;

    template <typename T, typename... Ts>
    constexpr bool are_same<T, Ts...> = std::conjunction_v<std::is_same<T, Ts>...>;

    template <field_c ... T> requires are_same<typename T::name_type::value_type...>
    struct message : private T... {

        message() = default;

        explicit message(T&& ...v) : T(std::move(v))... {};
        explicit message(const T& ...v) : T(v)... {};

        message(const message& other) : T(static_cast<const T&>(other))... {}
        message(message&& other) noexcept : T(static_cast<T&&>(other))... {}

        template <typename... U>
            requires (sizeof...(T) == sizeof...(U) && !are_same<message, std::remove_reference_t<U>...> )
        explicit message(U&& ...v) : T(std::forward<U>(v))... {};

        message& operator=(const message& other) {
            ((static_cast<T&>(*this) = static_cast<const T&>(other)), ...);
            return *this;
        }

        message& operator=(message&& other) noexcept {
            ((static_cast<T&>(*this) = static_cast<T&&>(other)), ...);
            return *this;
        }

        static constexpr uint<4> size = sizeof...(T);

        template <uint<4> N>
        using get_type_by_number = field_number_selector<N, T...>;

        template <basic_fixed_string S>
        using get_type_by_name = field_name_selector<S, T...>;

        template <uint<4> N>
        constexpr decltype(auto) get() const {
            return static_cast<const field_number_selector<N, T...>&>(*this);
        }

        template <basic_fixed_string S>
        constexpr decltype(auto) get() const {
            return static_cast<const field_name_selector<S, T...>&>(*this);
        }

        template <uint<4> N>
        constexpr decltype(auto) get() {
            return static_cast<field_number_selector<N, T...>&>(*this);
        }

        template <basic_fixed_string S>
        constexpr decltype(auto) get() {
            return static_cast<field_name_selector<S, T...>&>(*this);
        }

        template <uint<4> F>
        constexpr decltype(auto) operator[](std::integral_constant<uint<4>, F>) const {
            return get<F>();
        }

        template <basic_fixed_string F>
        constexpr decltype(auto) operator[](constant<F>) const {
            return get<F>();
        }

        template <uint<4> F>
        constexpr decltype(auto) operator[](std::integral_constant<uint<4>, F>) {
            return get<F>();
        }

        template <basic_fixed_string F>
        constexpr decltype(auto) operator[](constant<F>) {
            return get<F>();
        }

        template <uint<4> N>
        constexpr decltype(auto) get_base() const {
            return static_cast<const typename field_number_selector<N, T...>::base_type&>(get<N>());
        }

        template <basic_fixed_string S>
        constexpr decltype(auto) get_base() const {
            return static_cast<const typename field_name_selector<S, T...>::base_type&>(get<S>());
        }

        template <uint<4> N>
        constexpr decltype(auto) get_base() {
            return static_cast<typename field_number_selector<N, T...>::base_type&>(get<N>());
        }

        template <basic_fixed_string S>
        constexpr decltype(auto) get_base() {
            return static_cast<typename field_name_selector<S, T...>::base_type&>(get<S>());
        }

        constexpr bool operator==(const message & other) const {
            return ((
                    static_cast<const typename T::base_type&>(static_cast<const T&>(*this)) ==
                    static_cast<const typename T::base_type&>(static_cast<const T&>(other))
                    ) && ...);
        }

        constexpr bool operator!=(const message & other) const {
            return !(*this == other);
        }

        template <typename F> requires (std::invocable<F, T> && ...)
        constexpr void for_each(F&& f) const {
            (std::forward<F>(f)(static_cast<const T&>(*this)), ...);
        }

        template <typename F> requires (std::invocable<F, T> && ...)
        constexpr void for_each(F&& f) {
            (std::forward<F>(f)(static_cast<T&>(*this)), ...);
        }

    };

    template <typename>
    constexpr bool is_message = false;

    template <typename ...T>
    constexpr bool is_message <message<T...>> = true;

    template <typename T>
    concept message_c = is_message<T>;

    inline constexpr uint<1> to_wire_key(uint<4> field_key) {
        return field_key & 0b111u;
    }

    inline constexpr uint<4> to_field_number(uint<4> field_key) {
        return field_key >> 3u;
    }

    template <uint<1>>
    struct wire_skip_impl;

    template <>
    struct wire_skip_impl<0> {
        using type = skipper<varint_coder<uint<8>>>;
    };

    template <>
    struct wire_skip_impl<1> {
        using type = skipper<integer_coder<uint<8>>>;
    };

    template <>
    struct wire_skip_impl<2> {
        using type = skipper<string_coder>;
    };

    template <>
    struct wire_skip_impl<5> {
        using type = skipper<integer_coder<uint<4>>>;
    };

    template <uint<1> N>
    using wire_skip = typename wire_skip_impl<N>::type;

    template <uint<1>... I>
    struct message_skip_map_impl : std::unordered_map<uint<4>, std::function<bytes(bytes)>> {
        message_skip_map_impl() : std::unordered_map<uint<4>, std::function<bytes(bytes)>> {
                {I, [](bytes b){
                    return wire_skip<I>::decode_skip(b);
                }}...
        } {}
    };

    using message_skip_map = message_skip_map_impl<0, 1, 2, 5>;
    inline const message_skip_map skip_map;

    template <message_c>
    struct message_decode_map;

    template <field_c... F>
    struct message_decode_map<message<F...>> : std::unordered_map<uint<4>, std::function<bytes(message<F...>&, bytes)>> {
    private:
        using T = message<F...>;

    public:
        message_decode_map() : std::unordered_map<uint<4>, std::function<bytes(T&, bytes)>> {
                {F::key, [](T& m, bytes b){
                    const auto &[v, np] = F::coder::decode(b);

                    auto &f = m.template get<F::number>();
                    if constexpr (F::attr == singular) {
                        f = v;
                    } else {
                        *std::inserter(f, f.end()) = v;
                    }

                    return np;
                }}...
        } {}

        constexpr std::pair<bytes, bool> decode(T& v, bytes b) const {
            const auto &[n, nb] = varint_coder<uint<4>>::decode(b);

            if(to_field_number(n) == 0) {
                return {b, false};
            }

            auto iter = this->find(n);
            if (iter != this->end()) {
                b = iter->second(v, nb);
            } else {
                b = skip_map.at(to_wire_key(n))(nb);
            }

            return {b, true};
        }
    };

    template <message_c T>
    inline const message_decode_map<T> decode_map;

    template <message_c T>
    struct message_coder {
        using value_type = T;

        message_coder() = delete;

        static constexpr bytes encode(const T& msg, bytes b) {
            msg.for_each([&b]<field_c F> (const F& f) {
                if(empty_field(f)) {
                    return;
                }


                if constexpr (F::attr == singular) {
                    b = varint_coder<uint<4>>::encode(F::key, b);
                    b = F::coder::encode(f.value(), b);
                } else {
                    for(const auto &i : f) {
                        b = varint_coder<uint<4>>::encode(F::key, b);
                        b = F::coder::encode(i, b);
                    }
                }
            });

            return b;
        }

        static constexpr decode_result<T> decode(bytes b) {
            T v;

            while(b.end() > b.begin()) {
                bool next = true;
                std::tie(b, next) = decode_map<T>.decode(v, b);

                if(!next) break;
            }

            return {v, b};
        }
    };

    template <message_c T>
    struct skipper<message_coder<T>> {
        using value_type = T;

        static constexpr std::size_t encode_skip(const T& msg) {
            std::size_t n = 0;
            msg.for_each([&n]<field_c F> (const F& f) {
                if(empty_field(f)) {
                    return;
                }


                if constexpr (F::attr == singular) {
                    n += skipper<varint_coder<uint<4>>>::encode_skip(F::key);
                    n += skipper<typename F::coder>::encode_skip(f.value());
                } else {
                    for(const auto &i : f) {
                        n += skipper<varint_coder<uint<4>>>::encode_skip(F::key);
                        n += skipper<typename F::coder>::encode_skip(f.value());
                    }
                }
            });

            return n;
        }
    };

    template <message_c T>
    struct embedded_message_coder {
        using value_type = T;

        embedded_message_coder() = delete;

        static constexpr bytes encode(const T& msg, bytes b) {
            auto n = skipper<message_coder<T>>::encode_skip(msg);

            b = varint_coder<uint<8>>::encode(n, b);
            b = message_coder<T>::encode(msg, b);

            return b;
        }

        static constexpr decode_result<T> decode(bytes b) {
            T v;

            std::size_t len = 0;
            std::tie(len, b) = varint_coder<uint<8>>::decode(b);

            auto origin_b = b;
            while(begin_diff(b, origin_b) < len) {
                bool next = true;
                std::tie(b, next) = decode_map<T>.decode(v, b);

                if(!next) break;
            }

            return {v, b};
        }
    };

    template <typename T>
    struct wire_type_impl<embedded_message_coder<T>> : std::integral_constant<uint<1>, 2> {};

    template <basic_fixed_string S, uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using message_field = field<S, N, embedded_message_coder<T>, A, Container>;
}

#endif //PROTOPUF_MESSAGE_H
