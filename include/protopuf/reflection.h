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

    template<typename> inline constexpr bool always_false = false;

    template<typename... Ts> struct overloaded : Ts... { using Ts::operator()...; };
    template<typename... Ts> overloaded(Ts...) -> overloaded<Ts...>;

    template <field_c T>
    struct field_name {
        static constexpr inline auto value = T::name;
    };

    template <field_c T>
    struct field_number {
        static constexpr inline auto value = T::number;
    };

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

    template <template<typename> typename Prop, field_c T>
    using dynamic_key_type = typename dynamic_key_type_impl<Prop, T>::type;

    template <template<field_c> typename Prop, typename F, field_c ... Ts>
        requires std::conjunction_v<std::bool_constant<std::invocable<F, Ts>>...>
    auto dynamic_get_by(F&& f, const message<Ts...>& msg, const dynamic_key_type<Prop, type_get_first<Ts...>>& key) {
        using result_type = std::common_type_t<std::invoke_result_t<F, Ts>...>;
        using key_type = dynamic_key_type<Prop, type_get_first<Ts...>>;

        static const std::unordered_map <key_type, std::function<
            result_type (F&&, const message<Ts...>&)
        >> reflect_map = {
            {key_type { Prop<Ts>::value }, [](F&& f, const message<Ts...>& msg){
                return std::forward<F>(f)(msg.template get<Prop<Ts>::value>());
            }}...
        };

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

    template <typename F, message_c T>
    auto dynamic_get_by_name(F&& f, const T& msg, const auto& name) {
        return dynamic_get_by<field_name>(std::forward<F>(f), msg, name);
    }

    template <typename F, message_c T>
    auto dynamic_get_by_number(F&& f, const T& msg, const auto& number) {
        return dynamic_get_by<field_number>(std::forward<F>(f), msg, number);
    }
}

#endif //PROTOPUF_REFLECTION_H
