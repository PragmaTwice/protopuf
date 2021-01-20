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

    template <typename F, field_c ... Ts>
    auto dynamic_get_by_name(F&& f, const message<Ts...>& msg, const std::string& name) {
        using result_type = std::common_type_t<std::invoke_result_t<F, Ts>...>;

        static const std::unordered_map <std::string, std::function<
            result_type (F&&, const message<Ts...>&)
        >> reflect_map = {
            {std::string{ Ts::name.data }, [](F&& f, const message<Ts...>& msg){
                return std::forward<F>(f)(msg.template get<Ts::name>());
            }}...
        };

        if(auto iter = reflect_map.find(name); iter != reflect_map.end()) {
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
}

#endif //PROTOPUF_REFLECTION_H
