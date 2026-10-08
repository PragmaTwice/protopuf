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
#include "byte.h"
#include "coder_mode.h"

#include <unordered_map>
#include <functional>

namespace pp {

    /// Checks whether all types in the parameter list are equal
    template <typename...>
    constexpr inline bool are_same = true;

    template <typename T, typename... Ts>
    constexpr inline bool are_same<T, Ts...> = std::conjunction_v<std::is_same<T, Ts>...>;

    template <typename U, typename F>
    struct fold_impl {

        template <typename V> requires std::invocable<F, U&&, V&&>
        constexpr auto operator+(V&& other) {
            return pp::fold_impl {
                std::forward<F>(f)(static_cast<U&&>(v), std::forward<V>(other)), std::forward<F>(f)
            };
        }

        U v;
        F&& f;
    };

    template <typename U, typename F>
    fold_impl(U&&, F&&) -> fold_impl<U, F>;

    namespace type_forward_impl {
        template <typename V, typename W>
        using forward_const = std::conditional_t<std::is_const_v<W>, std::add_const_t<V>, V>;

        template <typename V, typename W>
        using forward_volatile = std::conditional_t<std::is_volatile_v<W>, std::add_volatile_t<V>, V>;

        template <typename V, typename W>
        using forward_cv = forward_volatile<forward_const<V, W>, W>;

        template <typename V, typename W>
        using forward_lvref = std::conditional_t<std::is_lvalue_reference_v<W>, std::add_lvalue_reference_t<V>, V>;

        template <typename V, typename W>
        using forward_rvref = std::conditional_t<std::is_rvalue_reference_v<W>, std::add_rvalue_reference_t<V>, V>;

        
        template <typename V, typename W>
        using forward_ref = forward_rvref<forward_lvref<V, W>, W>;

        template <typename T, typename U>
        using type = forward_ref<forward_cv<T, std::remove_reference_t<U>>, U>;
    }
    
    /// Copy cvref (`const`, `volatile` and lvalue/rvalue reference) of `U` to `T`
    template <typename T, typename U>
    using type_forward = type_forward_impl::type<T, U>;

    /// @brief The message type
    /// @param T the field types of the message, where all `T::name_type::value_type` are equal
    template <field_c ... T> requires are_same<typename T::name_type::value_type...>
    struct message : private T... {

        constexpr message() = default;

        constexpr explicit message(T&& ...v) : T(std::move(v))... {};
        constexpr explicit message(const T& ...v) : T(v)... {};

        constexpr message(const message& other) : T(static_cast<const T&>(other))... {}
        constexpr message(message&& other) noexcept : T(static_cast<T&&>(other))... {}

        template <typename... U>
            requires (sizeof...(T) == sizeof...(U) && !are_same<message, std::remove_reference_t<U>...> )
        constexpr explicit message(U&& ...v) : T(std::forward<U>(v))... {}

        constexpr message& operator=(const message& other) {
            ((static_cast<T&>(*this) = static_cast<const T&>(other)), ...);
            return *this;
        }

        constexpr message& operator=(message&& other) noexcept {
            ((static_cast<T&>(*this) = static_cast<T&&>(other)), ...);
            return *this;
        }

        /// the number of fields
        static constexpr uint<4> size = sizeof...(T);

        /// get the field type by the specific field number
        template <uint<4> N>
        using get_type_by_number = field_number_selector<N, T...>;

        /// get the field type by the specific field name
        template <basic_fixed_string S>
        using get_type_by_name = field_name_selector<S, T...>;

        /// get a field by the field number
        template <uint<4> N>
        constexpr decltype(auto) get() const {
            return static_cast<const field_number_selector<N, T...>&>(*this);
        }

        /// get a field by the field name
        template <basic_fixed_string S>
        constexpr decltype(auto) get() const {
            return static_cast<const field_name_selector<S, T...>&>(*this);
        }

        /// get a field by the field number
        template <uint<4> N>
        constexpr decltype(auto) get() {
            return static_cast<field_number_selector<N, T...>&>(*this);
        }

        /// get a field by the field name
        template <basic_fixed_string S>
        constexpr decltype(auto) get() {
            return static_cast<field_name_selector<S, T...>&>(*this);
        }

        /// get a field by the field number, i.e. `msg[233_i]`
        template <uint<4> F>
        constexpr decltype(auto) operator[](std::integral_constant<uint<4>, F>) const {
            return get<F>();
        }

        /// get a field by the field name, i.e. `msg["a"_f]`
        template <basic_fixed_string F>
        constexpr decltype(auto) operator[](constant<F>) const {
            return get<F>();
        }

        /// get a field by the field number, i.e. `msg[233_i]`
        template <uint<4> F>
        constexpr decltype(auto) operator[](std::integral_constant<uint<4>, F>) {
            return get<F>();
        }

        /// get a field by the field name, i.e. `msg["a"_f]
        template <basic_fixed_string F>
        constexpr decltype(auto) operator[](constant<F>) {
            return get<F>();
        }

        /// get a field as its @ref field::base_type by the field number
        template <uint<4> N>
        constexpr decltype(auto) get_base() const {
            return static_cast<const typename field_number_selector<N, T...>::base_type&>(get<N>());
        }

        /// get a field as its @ref field::base_type by the field name
        template <basic_fixed_string S>
        constexpr decltype(auto) get_base() const {
            return static_cast<const typename field_name_selector<S, T...>::base_type&>(get<S>());
        }

        /// get a field as its @ref field::base_type by the field number
        template <uint<4> N>
        constexpr decltype(auto) get_base() {
            return static_cast<typename field_number_selector<N, T...>::base_type&>(get<N>());
        }

        /// get a field as its @ref field::base_type by the field name
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

        /// iterate all fields and apply function `f` to them
        template <typename F> requires (std::invocable<F, T> && ...)
        constexpr void for_each(F&& f) const {
            (std::forward<F>(f)(static_cast<const T&>(*this)), ...);
        }

        /// iterate all fields and apply function `f` to them
        template <typename F> requires (std::invocable<F, T> && ...)
        constexpr void for_each(F&& f) {
            (std::forward<F>(f)(static_cast<T&>(*this)), ...);
        }

        /// same as `f(f(...f(f(init, field1), field2), ...), fieldN)`
        template <typename F, typename U>
        constexpr auto fold(F&& f, U&& init) const {
            return (fold_impl{ std::forward<U>(init), std::forward<F>(f) } + ... + static_cast<const T&>(*this)).v;
        }

        /// same as `f(f(...f(f(init, field1), field2), ...), fieldN)`
        template <typename F, typename U>
        constexpr auto fold(F&& f, U&& init) {
            return (fold_impl{ std::forward<U>(init), std::forward<F>(f) } + ... + static_cast<T&>(*this)).v;
        }

        /// @brief Merge another message into this message, for all fields: overwrite if it is singular and non-empty, merge to end otherwise
        ///
        /// same as `merge_field(field1, other.field1), ..., merge_field(fieldN, other.fieldN)`, ref to @ref merge_field
        template <merge_mode mode = merge_mode{}, typename M> requires std::same_as<std::remove_cvref_t<M>, message>
        constexpr void merge(M&& other) {
            (merge_field<mode>(static_cast<T&>(*this), static_cast<type_forward<T, M&&>>(other)), ...);
        }
    };

    /// Checks whether the type is a @ref message type
    template <typename>
    constexpr inline bool is_message = false;

    template <typename ...T>
    constexpr inline bool is_message <message<T...>> = true;

    /// A concept satisfied while type `T` is a @ref message type
    template <typename T>
    concept message_c = is_message<T>;

    /// Merge multiple messages into one message, and return the merged message
    template <merge_mode mode = merge_mode{}, typename M1, typename... MN>
    requires message_c<std::remove_cvref_t<M1>> && 
             are_same<std::remove_cvref_t<M1>, std::remove_cvref_t<MN>...>
    constexpr auto merge(M1&& msg1, MN&& ... msgN) {
        std::remove_cvref_t<M1> res = std::forward<M1>(msg1);

        (res.template merge<mode>(std::forward<MN>(msgN)), ...);

        return res;
    }

    /// Get the wire type from a field key, ref to @ref field::key and @ref wire_type
    inline constexpr uint<1> to_wire_key(uint<4> field_key) {
        return field_key & 0b111u;
    }

    /// Get the field number from a field key, ref to @ref field::key and @ref field::number
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

    /// Dispatch a wire type number to a @ref skipper
    template <uint<1> N>
    using wire_skip = typename wire_skip_impl<N>::type;

    template <coder_mode Mode, uint<1>... I>
    struct message_skip_map_impl : std::unordered_map<uint<4>, std::function<decode_skip_result<Mode>(bytes)>> {
        message_skip_map_impl() : std::unordered_map<uint<4>, std::function<decode_skip_result<Mode>(bytes)>> {
                {I, [](bytes b){
                    return wire_skip<I>::template decode_skip<Mode>(b);
                }}...
        } {}
    };

    template <coder_mode Mode = safe_mode>
    using message_skip_map = message_skip_map_impl<Mode, 0, 1, 2, 5>;
    template <coder_mode Mode = safe_mode>
    inline const message_skip_map<Mode> skip_map;

    template <coder_mode Mode = safe_mode>
    using message_decode_map_result = typename Mode::template result_type<std::pair<bytes, bool>>;

    template <coder_mode Mode = safe_mode>
    using message_decode_map_function_result = typename Mode::template result_type<bytes>;

    template <coder_mode, message_c>
    struct message_decode_map;

    template <coder_mode Mode, field_c... F>
    struct message_decode_map<Mode, message<F...>> :
        std::unordered_map<uint<4>, std::function<message_decode_map_function_result<Mode>(message<F...>&, bytes)>> {
    private:
        using T = message<F...>;
        using function_result = message_decode_map_function_result<Mode>;

    public:
        message_decode_map() : std::unordered_map<uint<4>, std::function<function_result(T&, bytes)>> {
                {F::key, [](T& m, bytes b){
                    decode_value<typename F::coder::value_type> decode_v;
                    if (Mode::get_value_from_result(F::coder::template decode<Mode>(b), decode_v)) {
                        auto &f = m.template get<F::number>();
                        push_field(f, std::move(decode_v.first));

                        return function_result{decode_v.second};
                    }
                    return function_result{};
                }}...
        } {}

        constexpr message_decode_map_result<Mode> decode(T& v, bytes b) const {
            decode_value<uint<4>> decode_v;
            if (!Mode::get_value_from_result(varint_coder<uint<4>>::decode<Mode>(b), decode_v)) {
                return {};
            }

            const auto &[n, nb] = decode_v;

            if(to_field_number(n) == 0) {
                return Mode::template make_result<message_decode_map_result<Mode>>(b, false);
            }

            const auto iter = this->find(n);
            if (iter != this->end()) {
                if (!Mode::get_value_from_result(iter->second(v, nb), b)) {
                    return {};
                }
            } else {
                if (!Mode::get_value_from_result(
                        skip_map<Mode>.at(to_wire_key(n))(nb), b)) {
                    return {};
                }
            }

            return Mode::template make_result<message_decode_map_result<Mode>>(b, true);
        }
    };

    template <coder_mode Mode, message_c T>
    inline const message_decode_map<Mode, T> decode_map;

    /// A @ref coder for @ref message type
    template <message_c T>
    struct message_coder {
        using value_type = T;

        message_coder() = delete;

        template <coder_mode Mode = safe_mode>
        static constexpr encode_result<Mode> encode(const T& msg, bytes b) {
            encode_result<Mode> result{b};
            msg.for_each([&result]<field_c F> (const F& f) {
                if(empty_field(f)) {
                    return;
                }

                bytes safe_b;
                if (!Mode::get_value_from_result(result, safe_b)) {
                    return;
                }

                if constexpr (F::attr == singular) {
                    result = varint_coder<uint<4>>::encode<Mode>(F::key, safe_b);
                    if (Mode::get_value_from_result(result, safe_b)) {
                        result = F::coder::template encode<Mode>(f.value(), safe_b);
                    }
                } else {
                    for(const auto &i : f) {
                        result = varint_coder<uint<4>>::encode<Mode>(F::key, safe_b);
                        if (Mode::get_value_from_result(result, safe_b)) {
                            result = F::coder::template encode<Mode>(i, safe_b);
                            if (!Mode::get_value_from_result(result, safe_b)) {
                                break;
                            }
                        } else {
                            break;
                        }
                    }
                }
            });

            return result;
        }
        
        template <coder_mode Mode = safe_mode>
        static constexpr decode_result<T, Mode> decode(bytes b) {
            T v;

            while(b.end() > b.begin()) {
                std::pair<bytes, bool> bytes_with_next;
                if (!Mode::get_value_from_result(decode_map<Mode, T>.decode(v, b), bytes_with_next)) {
                    return {};
                }

                bool next = true;
                std::tie(b, next) = bytes_with_next;

                if(!next) break;
            }

            return Mode::template make_result<decode_result<T, Mode>>(std::move(v), b);
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
                        n += skipper<typename F::coder>::encode_skip(i);
                    }
                }
            });

            return n;
        }
    };

    /// @brief A @ref coder for embedded message
    ///
    /// Ref to https://developers.google.com/protocol-buffers/docs/encoding#embedded
    template <message_c T>
    struct embedded_message_coder {
        using value_type = T;

        embedded_message_coder() = delete;

        template <coder_mode Mode = safe_mode>
        static constexpr encode_result<Mode> encode(const T& msg, bytes b) {
            auto n = skipper<message_coder<T>>::encode_skip(msg);

            if (Mode::get_value_from_result(varint_coder<uint<8>>::encode<Mode>(n, b), b)) {
                return message_coder<T>::template encode<Mode>(msg, b);
            }

            return {};
        }

        template <coder_mode Mode = safe_mode>
        static constexpr decode_result<T, Mode> decode(bytes b) {
            T v;

            decode_value<uint<8>> decod_len;
            if (!Mode::get_value_from_result(varint_coder<uint<8>>::decode<Mode>(b), decod_len)) {
                return {};
            }

            std::size_t len = 0;
            std::tie(len, b) = decod_len;

            while(len) {
                std::pair<bytes, bool> bytes_with_next;
                if (!Mode::get_value_from_result(decode_map<Mode, T>.decode(v, b), bytes_with_next)) {
                    return {};
                }

                --len;
                bool next = true;
                std::tie(b, next) = bytes_with_next;

                if(!next) break;
            }

            return Mode::template make_result<decode_result<T, Mode>>(std::move(v), b);
        }
    };

    template <typename T>
    struct skipper<embedded_message_coder<T>> {
        using value_type = T;

        static constexpr std::size_t encode_skip(const T& v) {
            uint<8> n = skipper<message_coder<T>>::encode_skip(v);

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

    template <typename T>
    struct wire_type_impl<embedded_message_coder<T>> : std::integral_constant<uint<1>, 2> {};

    /// Type alias for embedded message fields
    template <basic_fixed_string S, uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using message_field = field<S, N, embedded_message_coder<T>, A, Container>;
}

#endif //PROTOPUF_MESSAGE_H
