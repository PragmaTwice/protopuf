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

#ifndef PROTOPUF_MAP_H
#define PROTOPUF_MAP_H

#include "message.h"

namespace pp {

    template<coder key_coder, coder value_coder>
    using map_element_base = message<field<"key", 1, key_coder>, field<"value", 2, value_coder>>;

    template<coder key_coder, coder value_coder>
    struct map_element : map_element_base<key_coder, value_coder> {
        using base_type = map_element_base<key_coder, value_coder>;

        using first_field = typename base_type::template get_type_by_number<1>;
        using second_field = typename base_type::template get_type_by_number<2>;

        using first_type = typename first_field::base_type;
        using second_type = typename second_field::base_type;

        using pair_type = std::pair<const first_type, second_type>;

        map_element(const pair_type& v) : base_type(
            static_cast<const first_field &>(v.first), static_cast<const second_field &>(v.second)
        ) {}

        using base_type::base_type;

        operator pair_type() const {
            return pair_type { 
                this->template get<1>(), this->template get<2>() 
            };
        }
    };

    template <typename T1, typename T2>
    constexpr bool is_message <map_element<T1, T2>> = true;

    template <typename T1, typename T2>
    struct message_decode_map<map_element<T1, T2>> : message_decode_map<typename map_element<T1, T2>::base_type> {};

    template<basic_fixed_string S, uint<4> N, coder key_coder, coder value_coder,
        typename Container = std::map<
            typename map_element<key_coder, value_coder>::first_type, 
            typename map_element<key_coder, value_coder>::second_type
        >
    >
    using map_field = message_field<S, N, map_element<key_coder, value_coder>, repeated, Container>;

}

#endif //PROTOPUF_MAP_H
