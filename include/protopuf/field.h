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

#ifndef PROTOPUF_FIELD_H
#define PROTOPUF_FIELD_H

#include "int.h"
#include "float.h"
#include "array.h"
#include "zigzag.h"
#include "varint.h"
#include "bool.h"
#include "fixed_string.h"
#include <optional>

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

    template <>
    struct wire_type_impl<bool_coder> : std::integral_constant<uint<1>, 0> {};

    template <typename T>
    struct wire_type_impl<enum_coder<T>> : std::integral_constant<uint<1>, 0> {};

    /// @brief A wire type that provides just enough information to find the length of the following value
    /// @param T the input @ref coder type
    /// @returns A wire type number (in `uint<1>`) corresponding to the coder type
    ///
    /// Ref to https://developers.google.com/protocol-buffers/docs/encoding#structure
    template <typename T>
    constexpr uint<1> wire_type = wire_type_impl<T>::value;

    /// An attribute of the fields, to represent whether the field can repeat in message
    enum attribute {
        /// Represents a singular field, which can appear zero times or once in a message
        singular,
        /// Represents a repeated field, which can appear zero times, once or many times in a message
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

    /// @brief The underlying container type of a field
    /// @param A the @ref attribute of the field
    /// @param T the underlying object type (to store data) of the field
    /// @param Container the underlying container to store repeated objects, 
    /// it will be used while the attribute `A` is @ref repeated
    template <attribute A, typename T, std::ranges::sized_range Container = std::vector<T>>
    using field_container = typename field_container_impl<A, T, Container>::type;

    /// @brief The field type
    /// @param S the name of the field
    /// @param N the field number, ref to https://developers.google.com/protocol-buffers/docs/encoding#structure
    /// @param C the @ref coder of the field
    /// @param A the @ref attribute of the field
    /// @param Container the container used in @ref field_container, enabled while `A` is @ref repeated.
    template <basic_fixed_string S, uint<4> N, coder C, attribute A = singular, std::ranges::sized_range Container = std::vector<typename C::value_type>>
    struct field : field_container<A, typename C::value_type, Container>{
        /// name of the field
        static constexpr basic_fixed_string name = S;

        /// type of name of the field
        using name_type = decltype(name);

        /// the field number
        static constexpr uint<4> number = N;

        /// key of the field, ref to https://developers.google.com/protocol-buffers/docs/encoding#structure
        static constexpr uint<4> key = (N << 3u) | wire_type<C>;

        /// @ref coder of the field 
        using coder = C;

        /// attribute of the field
        static constexpr attribute attr = A;

        /// the underlying type (to store data of the field), which the field is derived from
        using base_type = field_container<A, typename C::value_type, Container>;

        using base_type::base_type;

        /// cast the field to @ref base_type
        constexpr decltype(auto) cast_to_base() {
            return static_cast<base_type&>(*this);
        }

        /// cast the const field to const @ref base_type
        constexpr decltype(auto) cast_to_base() const {
            return static_cast<const base_type&>(*this);
        }
    };

    /// Type alias for fixed-length integer fields
    template <basic_fixed_string S, uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using integer_field = field<S, N, integer_coder<T>, A, Container>;

    /// Type alias for fixed32 (fixed-length uint32) fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<uint<4>>>
    using fixed32_field = integer_field<S, N, uint<4>, A, Container>;

    /// Type alias for sfixed32 (fixed-length int32) fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<sint<4>>>
    using sfixed32_field = integer_field<S, N, sint<4>, A, Container>;

    /// Type alias for fixed64 (fixed-length uint64) fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<uint<8>>>
    using fixed64_field = integer_field<S, N, uint<8>, A, Container>;

    /// Type alias for sfixed64 (fixed-length int64) fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<sint<8>>>
    using sfixed64_field = integer_field<S, N, sint<8>, A, Container>;

    /// Type alias for varint fields
    template <basic_fixed_string S, uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using varint_field = field<S, N, varint_coder<T>, A, Container>;

    using int32 = sint<4>;
    /// Type alias for int32 (varint) fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<int32>>
    using int32_field = varint_field<S, N, int32, A, Container>;

    using uint32 = uint<4>;
    /// Type alias for uint32 (varint) fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<uint32>>
    using uint32_field = varint_field<S, N, uint32, A, Container>;

    using sint32 = sint_zigzag<4>;
    /// Type alias for sint32 (varint, Zigzag) fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<sint32>>
    using sint32_field = varint_field<S, N, sint32, A, Container>;

    using int64 = sint<8>;
    /// Type alias for int64 (varint) fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<int64>>
    using int64_field = varint_field<S, N, int64, A, Container>;

    using uint64 = uint<8>;
    /// Type alias for uint64 (varint) fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<uint64>>
    using uint64_field = varint_field<S, N, uint64, A, Container>;

    using sint64 = sint_zigzag<8>;
    /// Type alias for sint64 (varint, Zigzag) fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<sint64>>
    using sint64_field = varint_field<S, N, sint64, A, Container>;

    /// Type alias for floating point type fields
    template <basic_fixed_string S, uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using floating_field = field<S, N, float_coder<T>, A, Container>;

    /// Type alias for float fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<floating<4>>>
    using float_field = floating_field<S, N, floating<4>, A, Container>;

    /// Type alias for double fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<floating<8>>>
    using double_field = floating_field<S, N, floating<8>, A, Container>;

    /// Type alias for range type fields
    template <basic_fixed_string S, uint<4> N, coder T, attribute A = singular, typename Container = std::vector<typename T::value_type>>
    using array_field = field<S, N, array_coder<T>, A, Container>;

    /// Type alias for `std::basic_string` fields
    template <basic_fixed_string S, uint<4> N, typename T, attribute A = singular, typename Container = std::vector<std::basic_string<T>>>
    using basic_string_field = field<S, N, basic_string_coder<T>, A, Container>;

    /// Type alias for string fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<std::string>>
    using string_field = field<S, N, string_coder, A, Container>;

    /// Type alias for bytes fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<std::vector<std::byte>>>
    using bytes_field = field<S, N, bytes_coder, A, Container>;

    /// Type alias for boolean fields
    template <basic_fixed_string S, uint<4> N, attribute A = singular, typename Container = std::vector<bool>>
    using bool_field = field<S, N, bool_coder, A, Container>;

    /// Type alias for enumeration fields
    template <basic_fixed_string S, uint<4> N, typename T, attribute A = singular, typename Container = std::vector<T>>
    using enum_field = field<S, N, enum_coder<T>, A, Container>;

    /// Checks whether the type is a field type.
    template <typename>
    constexpr bool is_field = false;

    template <basic_fixed_string S, uint<4> N, coder C, attribute A, typename Container>
    constexpr bool is_field <field<S, N, C, A, Container>> = true;

    /// A concept satisfied while type `T` is a field type.
    template <typename T>
    concept field_c = is_field<T>;

    /// Represents the specific field is not found, used in @ref field_number_selector and @ref field_name_selector
    struct field_not_found;

    template <uint<4>, field_c...>
    struct field_number_selector_impl;

    template <uint<4> I, field_c C, field_c... D>
    struct field_number_selector_impl<I, C, D...> {
        using type = std::conditional_t<I == C::number, C, typename field_number_selector_impl<I, D...>::type>;
    };

    template <uint<4> I>
    struct field_number_selector_impl<I> {
        using type = field_not_found;
    };

    /// Find the first field in `C...` whose field number is `I`
    template <uint<4> I, field_c... C>
    using field_number_selector = typename field_number_selector_impl<I, C...>::type;

    template <basic_fixed_string, field_c...>
    struct field_name_selector_impl;

    template <basic_fixed_string S, field_c C, field_c... D>
    struct field_name_selector_impl<S, C, D...> {
        using type = std::conditional_t<S == C::name, C, typename field_name_selector_impl<S, D...>::type>;
    };

    template <basic_fixed_string S>
    struct field_name_selector_impl<S> {
        using type = field_not_found;
    };

    /// Find the first field in `C...` whose field name is `S`
    template <basic_fixed_string S, field_c... C>
    using field_name_selector = typename field_name_selector_impl<S, C...>::type;

    /// Checks whether a field is empty
    template <field_c T>
    constexpr bool empty_field(const T& v) {
        if constexpr (T::attr == singular) {
            return !v.has_value();
        } else {
            return v.empty();
        }
    }

    /// Push a value into a field: overwrite if it is singular, insert to end otherwise
    template <field_c F, typename T>
    constexpr void push_field(F& f, T&& v) {
        if constexpr (F::attr == singular) {
            f = std::forward<T>(v);
        } else {
            *std::inserter(f, f.end()) = std::forward<T>(v);
        }
    }

    /// Merge a field into another field: overwrite if it is singular and non-empty, merge to end otherwise
    template <field_c D, typename S> requires field_c<std::remove_cvref_t<S>>
    constexpr void merge_field(D& f, S&& v) {
        if constexpr (D::attr == singular) {
            if (!empty_field(v)) {
                f = std::forward<S>(v);
            }
        } else {
            auto inserter = std::inserter(f, f.end());

            if constexpr (std::is_rvalue_reference_v<S&&>) {
                std::move(v.begin(), v.end(), inserter);
            } else {
                std::copy(v.begin(), v.end(), inserter);
            }
        }
    }

    template <uint<4> V, char ... D> requires (('0' <= D <= '9') && ...)
    constexpr auto field_literal_helper = V;

    template <uint<4> V, char D1, char ... Dn>
    constexpr auto field_literal_helper<V, D1, Dn...> = field_literal_helper<V * 10 + (D1 - '0'), Dn...>;

    /// Constant integer literals, i.e. `233_i` as `std::integral_constant<uint<4>, 233>`
    template <char ... D>
    constexpr auto operator"" _i() {
        return std::integral_constant<uint<4>, field_literal_helper<0, D...>>{};
    }
}

#endif //PROTOPUF_FIELD_H
