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

#ifndef PROTOPUF_REFLECTION_H
#define PROTOPUF_REFLECTION_H

#include "message.h"

namespace pp {

    /// Make `false` a dependent name
    template<typename> inline constexpr bool always_false = false;

    /// @brief An overload method for lambda expression
    ///
    /// Ref to https://en.cppreference.com/w/cpp/utility/variant/visit#Example
    template<typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    /// A getter type for field name
    template <field_c T>
    struct field_name {
        static constexpr inline auto value = T::name;
    };

    /// A getter type for field number
    template <field_c T>
    struct field_number {
        static constexpr inline auto value = T::number;
    };

    /// Get the first type from a type list `Ts...`
    template <typename... Ts> requires (sizeof...(Ts) > 0)
    using type_get_first = type_get<0, Ts...>;

    template <template<typename> typename, field_c>
    struct dynamic_key_type_impl;

    template <field_c T>
    struct dynamic_key_type_impl<field_name, T> {
        using type = std::basic_string_view<typename decltype(field_name<T>::value)::value_type>;
    };

    template <field_c T>
    struct dynamic_key_type_impl<field_number, T> {
        using type = std::remove_cv_t<decltype(field_number<T>::value)>;
    };

    /// @brief Get the run-time type for a field number/name, i.e. a `std::basic_string_view` for field name
    /// @param Prop the getter, which can be @ref field_name or @ref field_number
    /// @param T the field type
    template <template<typename> typename Prop, field_c T>
    using dynamic_key_type = typename dynamic_key_type_impl<Prop, T>::type;

    template <template<typename> typename Prop, typename F, template <typename ...> typename M, typename... Ts>
        requires std::conjunction_v<std::bool_constant<std::invocable<F, Ts>>...> && message_c<std::remove_cv_t<M<Ts...>>>
    struct dynamic_visit_impl {

        using message_type = M<Ts...>;
        using result_type = std::common_type_t<std::invoke_result_t<F, Ts>...>;
        using key_type = dynamic_key_type<Prop, type_get_first<Ts...>>;

        static inline const std::unordered_map <key_type, std::function<
            result_type (F&&, message_type&)
        >> reflect_map = {
            {key_type { Prop<Ts>::value }, [](F&& f, message_type& msg){
                return std::forward<F>(f)(msg.template get<Prop<Ts>::value>());
            }}...
        };

        static auto visit(F&& f, message_type& msg, const key_type& key) {
            if(auto iter = reflect_map.find(key); iter != reflect_map.end()) {
                if constexpr (std::is_same_v<result_type, void>) {
                    iter->second(std::forward<F>(f), msg);
                    return true;
                } else {
                    return std::optional{iter->second(std::forward<F>(f), msg)};
                }
            }

            if constexpr (std::is_same_v<result_type, void>) {
                return false;
            } else {
                return std::optional<result_type>{};
            }
        }
    };

    /// A const wrapper of @ref message
    template <typename ...T>
    using const_message = const message<T...>;

    /// @brief Apply a function `f` to a field found by a property with run-time value `key` in a const-qualified message `msg`
    /// 
    /// Template parameters:
    /// @param Prop the getter, which can be @ref field_name or @ref field_number
    /// 
    /// Function parameters:
    /// @param f the callback function to be invoked
    /// @param msg the message where the field is found 
    /// @param key the run-time value to find the specific field in the message
    template <template<field_c> typename Prop, typename F, field_c ... Ts>
    auto dynamic_visit_by(F&& f, const message<Ts...>& msg, const dynamic_key_type<Prop, type_get_first<Ts...>>& key) {
        return dynamic_visit_impl<Prop, F, const_message, Ts...>::visit(std::forward<F>(f), msg, key);
    }
    
    /// @brief Apply a function `f` to a field found by a property with run-time value `key` in a non-const message `msg`
    /// 
    /// Template parameters:
    /// @param Prop the getter, which can be @ref field_name or @ref field_number
    /// 
    /// Function parameters:
    /// @param f the callback function to be invoked
    /// @param msg the message where the field is found 
    /// @param key the run-time value to find the specific field in the message
    template <template<field_c> typename Prop, typename F, field_c ... Ts>
    auto dynamic_visit_by(F&& f, message<Ts...>& msg, const dynamic_key_type<Prop, type_get_first<Ts...>>& key) {
        return dynamic_visit_impl<Prop, F, message, Ts...>::visit(std::forward<F>(f), msg, key);
    }

    /// Apply a function `f` to a field found by run-time field `name` in a message `msg`, same as `dynamic_visit_by<field_name>`
    template <typename F,  typename T> requires message_c<std::remove_cvref_t<T>>
    auto dynamic_visit_by_name(F&& f, T&& msg, const auto& name) {
        return dynamic_visit_by<field_name>(std::forward<F>(f), std::forward<T>(msg), name);
    }

    /// Apply a function `f` to a field found by run-time field `number` in a message `msg`, same as `dynamic_visit_by<field_number>`
    template <typename F, typename T> requires message_c<std::remove_cvref_t<T>>
    auto dynamic_visit_by_number(F&& f, T&& msg, const auto& number) {
        return dynamic_visit_by<field_number>(std::forward<F>(f), std::forward<T>(msg), number);
    }
}

#endif //PROTOPUF_REFLECTION_H
