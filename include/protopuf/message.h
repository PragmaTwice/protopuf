#ifndef PROTOPUF_MESSAGE_H
#define PROTOPUF_MESSAGE_H

#include "coder.h"
#include "int.h"
#include "varint.h"
#include "zigzag.h"
#include "array.h"
#include "float.h"

namespace pp {
    template <coder T>
    constexpr uint<1> wire_type = -1;

    template <typename T>  requires integral32<T> || integral64<T>
    constexpr uint<1> wire_type<varint_coder<T>> = 0;

    template <integral64 T>
    constexpr uint<1> wire_type<integer_coder<T>> = 1;

    template <floating_point64 T>
    constexpr uint<1> wire_type<float_coder<T>> = 1;

    template <typename T, typename C>
    constexpr uint<1> wire_type<array_coder<T, C>> = 2;

    template <integral32 T>
    constexpr uint<1> wire_type<integer_coder<T>> = 5;

    template <floating_point32 T>
    constexpr uint<1> wire_type<float_coder<T>> = 5;

    enum attribute {
        singular,
        repeated
    };

    template <attribute, typename T, typename>
    struct field_container_impl {
        using type = std::optional<T>;
    };

    template <typename T, typename C>
    struct field_container_impl<repeated, T, C> {
        using type = C;
    };

    template <attribute A, typename T, insertable_sized_range Container = std::vector<T>>
    using field_container = typename field_container_impl<A, T, Container>::type;

    template <uint<4> N, coder C, attribute A = singular, insertable_sized_range Container = std::vector<typename C::value_type>>
    struct field : field_container<A, typename C::value_type, Container>{
        static constexpr uint<4> number = N;

        static constexpr uint<4> key = (N << 3) | wire_type<C>;

        using coder = C;

        static constexpr attribute attr = A;

        using base_type = field_container<A, typename C::value_type, Container>;

        using base_type::base_type;
    };

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using integer_field = field<N, integer_coder<T>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using varint_field = field<N, varint_coder<T>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using float_field = field<N, float_coder<T>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using array_field = field<N, array_coder<T, Container>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular>
    using basic_string_field = field<N, basic_string_coder<T>, A, std::basic_string<T>>;

    template <uint<4> N, attribute A = singular>
    using string_field = field<N, string_coder, A, std::string>;

    template <uint<4> N, attribute A = singular>
    using bytes_field = field<N, bytes_coder, A>;

    template <typename>
    constexpr bool is_field = false;

    template <uint<4> N, coder C, attribute A, typename Container>
    constexpr bool is_field <field<N, C, A, Container>> = true;

    template <typename T>
    concept field_c = is_field<T>;

    struct field_not_found;

    template <uint<4>, field_c...>
    struct field_selector_impl;

    template <uint<4> I, field_c C, field_c... D>
    struct field_selector_impl<I, C, D...> {
        using type = std::conditional_t<I == C::number, C, typename field_selector_impl<I, D...>::type>;
    };

    template <uint<4> I>
    struct field_selector_impl<I> {
        using type = field_not_found;
    };

    template <uint<4> I, field_c... C>
    using field_selector = typename field_selector_impl<I, C...>::type;

    template <field_c T>
    constexpr bool empty_field(const T& v) {
        if constexpr (T::attr == singular) {
            return !v.has_value();
        } else {
            return v.empty();
        }
    };

    template <field_c ... T>
    struct message : private T... {

        message() = default;

        explicit message(T&& ...v) : T(std::forward<T>(v))... {};

        message(const message& other) : T(static_cast<const T&>(other))... {}

        static constexpr uint<4> size = sizeof...(T);

        template <uint<4> N>
        constexpr decltype(auto) get() const {
            return static_cast<const field_selector<N, T...>&>(*this);
        }

        template <uint<4> N>
        constexpr decltype(auto) get() {
            return static_cast<field_selector<N, T...>&>(*this);
        }

        template <uint<4> N>
        constexpr decltype(auto) get_base() const {
            return static_cast<const typename field_selector<N, T...>::base_type&>(get<N>());
        }

        template <uint<4> N>
        constexpr decltype(auto) get_base() {
            return static_cast<typename field_selector<N, T...>::base_type&>(get<N>());
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

    template <message_c T>
    struct message_coder {
        using value_type = T;

        message_coder() = delete;

        static constexpr bytes encode(const T& msg, bytes b) {
            msg.for_each([&b]<field_c F> (const F& f) {
                if(empty_field(f)) {
                    return;
                }

                b = varint_coder<uint<4>>::encode(F::key, b);

                if constexpr (F::attr == singular) {
                    b = F::coder::encode(f.value(), b);
                } else {
                    for(const auto &i : f) {
                        b = F::coder::encode(i, b);
                    }
                }
            });

            return b;
        }

        static constexpr decode_result<T> decode(bytes b) {
            return {{}, 0};
        }
    };
}

#endif //PROTOPUF_MESSAGE_H
