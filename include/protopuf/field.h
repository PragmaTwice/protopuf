#ifndef PROTOPUF_FIELD_H
#define PROTOPUF_FIELD_H

#include "int.h"
#include "float.h"
#include "array.h"
#include "zigzag.h"

namespace pp {

    template <coder T>
    struct wire_type_impl;

    template <typename T>  requires integral32<T> || integral64<T>
    struct wire_type_impl<varint_coder<T>> : std::integral_constant<uint<1>, 0> {};

    template <integral64 T>
    struct wire_type_impl<integer_coder<T>> : std::integral_constant<uint<1>, 1> {};

    template <floating_point64 T>
    struct wire_type_impl<float_coder<T>> : std::integral_constant<uint<1>, 1> {};

    template <typename T, typename C>
    struct wire_type_impl<array_coder<T, C>> : std::integral_constant<uint<1>, 2> {};

    template <integral32 T>
    struct wire_type_impl<integer_coder<T>> : std::integral_constant<uint<1>, 5> {};

    template <floating_point32 T>
    struct wire_type_impl<float_coder<T>> : std::integral_constant<uint<1>, 5> {};

    template <typename T>
    constexpr uint<1> wire_type = wire_type_impl<T>::value;

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

    template <attribute A, typename T, std::ranges::sized_range Container = std::vector<T>>
    using field_container = typename field_container_impl<A, T, Container>::type;

    template <uint<4> N, coder C, attribute A = singular, std::ranges::sized_range Container = std::vector<typename C::value_type>>
    struct field : field_container<A, typename C::value_type, Container>{
        static constexpr uint<4> number = N;

        static constexpr uint<4> key = (N << 3u) | wire_type<C>;

        using coder = C;

        static constexpr attribute attr = A;

        using base_type = field_container<A, typename C::value_type, Container>;

        using base_type::base_type;
    };

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using integer_field = field<N, integer_coder<T>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<uint<4>>>
    using fixed32_field = integer_field<N, uint<4>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<sint<4>>>
    using sfixed32_field = integer_field<N, sint<4>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<uint<8>>>
    using fixed64_field = integer_field<N, uint<8>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<sint<8>>>
    using sfixed64_field = integer_field<N, sint<8>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using varint_field = field<N, varint_coder<T>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<sint<4>>>
    using int32_field = varint_field<N, sint<4>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<uint<4>>>
    using uint32_field = varint_field<N, uint<4>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<sint_zigzag<4>>>
    using sint32_field = varint_field<N, sint_zigzag<4>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<sint<8>>>
    using int64_field = varint_field<N, sint<8>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<uint<8>>>
    using uint64_field = varint_field<N, uint<8>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<sint_zigzag<8>>>
    using sint64_field = varint_field<N, sint_zigzag<8>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using floating_field = field<N, float_coder<T>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<floating<4>>>
    using float_field = floating_field<N, floating<4>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<floating<8>>>
    using double_field = floating_field<N, floating<8>, A, Container>;

    template <uint<4> N, coder T, attribute A = singular, typename Container = std::vector<typename T::value_type>>
    using array_field = field<N, array_coder<T>, A, Container>;

    template <uint<4> N, typename T, attribute A = singular, typename Container = std::vector<std::basic_string<T>>>
    using basic_string_field = field<N, basic_string_coder<T>, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<std::string>>
    using string_field = field<N, string_coder, A, Container>;

    template <uint<4> N, attribute A = singular, typename Container = std::vector<std::vector<bytes>>>
    using bytes_field = field<N, bytes_coder, A, Container>;

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
    }

    template <uint<4> V, char ... D> requires (('0' <= D <= '9') && ...)
    constexpr auto field_literal_helper = V;

    template <uint<4> V, char D1, char ... Dn>
    constexpr auto field_literal_helper<V, D1, Dn...> = field_literal_helper<V * 10 + (D1 - '0'), Dn...>;

    template <char ... D>
    constexpr auto operator"" _f() {
        return std::integral_constant<uint<4>, field_literal_helper<0, D...>>{};
    }
}

#endif //PROTOPUF_FIELD_H
